#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "sitegen/markless/parser.h"

typedef struct {
	enum {
		ML_DT_ROOT,
		ML_DT_COUNT,
	} type;
	union {};
} markless_directive;

typedef struct {
	markless_directive directive;
	markless_component component;
} stack_entry;

typedef struct {
	sourcebuffer source;
	enum {
		LB_MODE_SHOW,
		LB_MODE_HIDE
	} linebreak_mode;
	size_t cursor;
	vector(stack_entry) stack;
	vector(stringview) disabled_directives;
	// fuck we need a table
	// **fuck**
	// uhhhhh no labels for now fuck u
} parser_state;

static bool match_directive(parser_state* parser, markless_directive directive, size_t* advance_by) {
	_Static_assert(ML_DT_COUNT == 1, "non-exhaustive: match_directive");
	switch (directive.type) {
		// always matches, never applicable
		case ML_DT_ROOT:
			return true;
		case ML_DT_COUNT:
			fprintf(stderr, "unreachable");
			exit(1);
	}
	assert(false && "TODO!");
}

static uint32_t peek_char(parser_state* parser, size_t byte_offset, size_t* char_size) {
	// read in a utf-8 encoded codepoint, as well as transforming ``\r``, ``\r\n`` and ``\n`` into "newlines" (``\n``), and escaping characters with ``\\``
	// "escaped" characters are the same as their regular codepoint but or-d with 0x80000000 (top bit set)
	size_t peek_idx = parser->cursor + byte_offset;

	uint32_t character = 0;

	uint8_t header = (uint8_t)parser->source.data.data[peek_idx];

	if ((header & 0x80) == 0x00) {
		// ascii, yippee!
		// do we have a backslash?
		if (header == '\\') {
			// we do!
			// let's peek the next one and escape it
			character = 0x80000000 | peek_char(parser, 1, char_size);
			if (char_size)
				(*char_size)++; // go one bigger for the ``\``

			return character;
		}
		// not a backslash, do we have ``\r``?
		if (header == '\r') {
			// we do!
			// let's peek the next byte and see if it's just a ``\n``, this matters for consuming
			if (peek_idx + 1 >= parser->source.data.len) {
				// no further data, so just quit while we're ahead
				if (char_size)
					*char_size = 1;
				return L'\n';
			}
			uint8_t next = (uint8_t)parser->source.data.data[peek_idx+1];
			if (next == '\n') {
				// ``\r\n``, 2 byte long newline
				if (char_size)
					*char_size = 2;
				return L'\n';
			}
			if (char_size) *char_size = 1;
			return L'\n';
		}
		// not a carriage return either, do we have ``\n``?
		if (header == '\n') {
			// we do!
			if (char_size)
				*char_size = 1;
			return L'\n';
		}
		// otherwise, we have an ascii value, return it
		if (char_size)
			*char_size = 1;

		return header;
	}

	// utf-8 header?
	// let's make sure it's not a follower
	if ((header & 0xC0) == 0x80) {
		// fuck
		fprintf(stderr, "unexpected utf-8 follower byte\n");
		exit(1);
	}
	size_t byte_count = 0;
	// the byte count of a utf-8 sequence = the amount of most-significant-bits set
	for (uint8_t shifted = header; (shifted & 0x80) != 0; shifted <<= 1, byte_count++);

	size_t follower_count = byte_count - 1;
	size_t bits_to_ignore = byte_count + 2;

	peek_idx++;

	character = header & (0xFF >> bits_to_ignore);

	for (size_t off = 0; off < follower_count; off++, peek_idx++) {
		if (peek_idx >= parser->source.data.len) {
			fprintf(stderr, "too few remaining bytes to fulfill follower byte requirement\n");
			exit(1);
		}
		uint8_t follower = parser->source.data.data[peek_idx];
		if ((follower & 0xC0) != 0x80) {
			fprintf(stderr, "expected utf-8 follower byte\n");
			exit(1);
		}
		character <<= 6;
		character |= follower & 0x3F;
	}

	if (char_size)
		*char_size = byte_count;

	return character;
}

static bool at_end_of_line(parser_state* parser) {
	uint32_t peeked = peek_char(parser, 0, NULL);

	return peeked == '\n';
}

markless_doc* parse_markless_document(sitegen_context* context, sourcebuffer source) {
	markless_doc* document = (markless_doc*)malloc(sizeof(markless_doc));
	parser_state* parser   = (parser_state*)malloc(sizeof(parser_state));
	memset(document, 0, sizeof(markless_doc));
	memset(parser,   0, sizeof(parser_state));

	parser->source = source;
	parser->linebreak_mode;

	stack_entry root_entry = (stack_entry){
		.directive = (markless_directive){
			.type = ML_DT_ROOT,
		},
		.component = (markless_component){
			.type = ML_CT_ROOT_DOCUMENT,
			.root = document,
		},
	};

	vector_push(parser->stack, root_entry);

	while (parser->cursor < parser->source.data.len) {
		int current_entry;
		for (current_entry = 0; current_entry < vector_count(parser->stack); current_entry++) {
			size_t advance_by = 0;
			if (match_directive(parser, parser->stack[current_entry].directive, &advance_by)) {
				parser->cursor += advance_by;
				continue; // all good
			} else {
				// stack unwind
				assert(false && "TODO: stack unwinding");
				break;
			}
		}

		while (!at_end_of_line(parser)) {
			// invoke
			assert(false && "TODO: invoking");
		}

		assert(false && "TODO!");
	}

	return document;
}
