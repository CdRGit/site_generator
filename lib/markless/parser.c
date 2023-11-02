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
		ML_DT_HEADER,
		ML_DT_PARAGRAPH,
		ML_DT_BLOCKQUOTE_BODY,
		ML_DT_BLOCKQUOTE_HEADER,
		ML_DT_ORDERED_LIST_ITEM,
		ML_DT_UNORDERED_LIST_ITEM,
		ML_DT_HORIZONTAL_RULE,
		ML_DT_COUNT,
	} type;
	union {
		size_t depth;
	};
} markless_directive;

typedef struct {
	markless_directive  directive;
	markless_component* component;
} stack_entry;

typedef struct {
	sourcebuffer source;
	enum {
		LB_MODE_SHOW,
		LB_MODE_HIDE
	} linebreak_mode;
	bool allow_paragraph;
	bool inline_only;
	bool blockquote_body_special_case;
	enum {
		LI_NONE,
		LI_ORDERED,
		LI_UNORDERED
	} must_match_new_list_item_or_list_ends;
	size_t cursor;
	vector(stack_entry) stack;
	vector(stringview) disabled_directives;
	// fuck we need a table
	// **fuck**
	// uhhhhh no labels for now fuck u
} parser_state;

static markless_component* final_component(vector(markless_component)* components) {
	if (vector_count(components) == 0) return NULL;
	return *vector_back(components);
}

static markless_component* get_last_child(markless_component* component) {
	_Static_assert(ML_CT_COUNT == 12, "non-exhaustive: get_last_child");
	switch (component->type) {
		case ML_CT_ROOT_DOCUMENT:
			return final_component(component->root->children);
		case ML_CT_HEADER:
			return final_component(component->header->children);
		case ML_CT_PARAGRAPH:
			return final_component(component->paragraph->children);
		case ML_CT_BLOCKQUOTE_BODY:
			return final_component(component->blockquote_body->children);
		case ML_CT_BLOCKQUOTE_HEADER:
			return final_component(component->blockquote_header->children);
		case ML_CT_ORDERED_LIST:
			return final_component(component->ordered_list->items);
		case ML_CT_ORDERED_LIST_ITEM:
			return final_component(component->ordered_list_item->children);
		case ML_CT_UNORDERED_LIST:
			return final_component(component->unordered_list->items);
		case ML_CT_UNORDERED_LIST_ITEM:
			return final_component(component->unordered_list_item->children);
		case ML_CT_TEXT:
		case ML_CT_NEWLINE:
		case ML_CT_HORIZONTAL_RULE:
			fprintf(stderr, "component has no children");
			exit(1);
		case ML_CT_COUNT:
			fprintf(stderr, "unreachable");
			exit(1);
	}
	assert(false && "TODO!");
	return NULL;
}

static markless_component* get_or_add_text_child(vector(markless_component*)* children) {
	if (*children && vector_count(*children) > 0) {
		size_t idx = (size_t)vector_count(*children) - 1;
		markless_component* potential = (*children)[idx];
		if (potential->type == ML_CT_TEXT) return potential;
		goto add;
	}
add:{
		markless_component* new_child = (markless_component*)malloc(sizeof(markless_component));
		new_child->type = ML_CT_TEXT;
		markless_text* text = (markless_text*)malloc(sizeof(markless_text));
		memset(text, 0, sizeof(markless_text));
		new_child->text = text;
		vector_push(*children, new_child);
		return get_or_add_text_child(children);
	}
}

static void add_char_to_char8_vector(vector(char)* vec, uint32_t c) {
	c = c & 0x7FFFFFFF;
	if (c < 0x80) {
		// ASCII, EZ
		vector_push(*vec, (char)c);
	} else {
		// outside of ASCII, less EZ
		assert(c <= 0x10FFFF && "outside of unicode range! WTF!?");
		if (c <= 0x7FF) {
			// 2-byte
			vector_push(*vec, (char)(0xC0 | (0x1F & (c >>  6))));
			vector_push(*vec, (char)(0x80 | (0x3F & (c >>  0))));
		} else if (c <= 0xFFFF) {
			// 3-byte
			vector_push(*vec, (char)(0xE0 | (0x0F & (c >> 12))));
			vector_push(*vec, (char)(0x80 | (0x3F & (c >>  6))));
			vector_push(*vec, (char)(0x80 | (0x3F & (c >>  0))));
		} else if (c <= 0x1FFFFF) {
			// 4-byte
			vector_push(*vec, (char)(0xF0 | (0x07 & (c >> 18))));
			vector_push(*vec, (char)(0x80 | (0x3F & (c >> 12))));
			vector_push(*vec, (char)(0x80 | (0x3F & (c >>  6))));
			vector_push(*vec, (char)(0x80 | (0x3F & (c >>  0))));
		} else {
			assert(false && "TODO! UTF-8 sequences");
		}
	}
}

static void add_char_to_component(markless_component* component, uint32_t c) {
	_Static_assert(ML_CT_COUNT == 12, "non-exhaustive: add_char_to_component");
	switch (component->type) {
		case ML_CT_ROOT_DOCUMENT:
			// NOP
			return;
		case ML_CT_HEADER:
			add_char_to_component(get_or_add_text_child(&component->header->children), c);
			return;
		case ML_CT_PARAGRAPH:
			add_char_to_component(get_or_add_text_child(&component->paragraph->children), c);
			return;
		case ML_CT_BLOCKQUOTE_BODY:
			add_char_to_component(get_or_add_text_child(&component->blockquote_body->children), c);
			return;
		case ML_CT_BLOCKQUOTE_HEADER:
			add_char_to_component(get_or_add_text_child(&component->blockquote_header->children), c);
			return;
		case ML_CT_ORDERED_LIST:
			// NOP
			return;
		case ML_CT_ORDERED_LIST_ITEM:
			add_char_to_component(get_or_add_text_child(&component->ordered_list_item->children), c);
			return;
		case ML_CT_UNORDERED_LIST:
			// NOP
			return;
		case ML_CT_UNORDERED_LIST_ITEM:
			add_char_to_component(get_or_add_text_child(&component->unordered_list_item->children), c);
			return;
		case ML_CT_TEXT:
			add_char_to_char8_vector(&component->text->text, c);
			return;
		case ML_CT_NEWLINE:
			fprintf(stderr, "cannot add character to newline entity");
			exit(1);
		case ML_CT_HORIZONTAL_RULE:
			fprintf(stderr, "cannot add character to horizontal rule");
			exit(1);
		case ML_CT_COUNT:
			fprintf(stderr, "unreachable");
			exit(1);
	}
	assert(false && "TODO!");
}

static uint32_t peek_char(parser_state* parser, size_t byte_offset, size_t* char_size, bool allow_escapes) {
	// read in a utf-8 encoded codepoint, as well as transforming ``\r``, ``\r\n`` and ``\n`` into "newlines" (``\n``), and escaping characters with ``\\``
	// "escaped" characters are the same as their regular codepoint but or-d with 0x80000000 (top bit set)
	size_t peek_idx = parser->cursor + byte_offset;
	// null terminator
	if (peek_idx >= parser->source.data.len) return 0;

	uint32_t character = 0;

	uint8_t header = (uint8_t)parser->source.data.data[peek_idx];

	if ((header & 0x80) == 0x00) {
		// ascii, yippee!
		// do we have a backslash?
		if (header == '\\') {
			// we do!
			// let's peek the next one and escape it
			character = 0x80000000 | peek_char(parser, 1, char_size, false);
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
		uint8_t follower = (uint8_t)parser->source.data.data[peek_idx];
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
	uint32_t peeked = peek_char(parser, 0, NULL, true);

	return peeked == '\n';
}

static bool at_whitespace(parser_state* parser, size_t offset) {
	uint32_t peeked = peek_char(parser, offset, NULL, true);
	return (peeked == ' ' || peeked == '\t' || peeked == '\n');
}

static size_t count_whitespace_limit(parser_state* parser, size_t* offset, size_t limit) {
	size_t depth = 0;
	size_t off = 0;
	while (depth < limit) {
		size_t advance_by;
		uint32_t peeked = peek_char(parser, off, &advance_by, true);
		if (peeked == ' ') {
			off += advance_by;
			depth++;
			continue;
		} else if (peeked == '\t') {
			off += advance_by;
			depth += 4 - (depth % 4);
			continue;
		}
		break;
	}
	if (offset)
		*offset = off;
	return depth;
}
static size_t count_whitespace(parser_state* parser, size_t* offset) {
	size_t depth = 0;
	size_t off = 0;
	while (true) {
		size_t advance_by;
		uint32_t peeked = peek_char(parser, off, &advance_by, true);
		if (peeked == ' ') {
			off += advance_by;
			depth++;
			continue;
		} else if (peeked == '\t') {
			off += advance_by;
			depth += 4 - (depth % 4);
			continue;
		}
		break;
	}
	if (offset)
		*offset = off;
	return depth;
}

static size_t consume_whitespace(parser_state* parser) {
	size_t depth = 0;
	while (true) {
		size_t advance_by;
		uint32_t peeked = peek_char(parser, 0, &advance_by, true);
		if (peeked == ' ') {
			parser->cursor += advance_by;
			depth++;
			continue;
		} else if (peeked == '\t') {
			parser->cursor += advance_by;
			depth += 4 - (depth % 4);
			continue;
		}
		break;
	}
	return depth;
}

static bool match_directive(parser_state* parser, markless_directive directive, size_t* advance_by) {
	_Static_assert(ML_DT_COUNT == 8, "non-exhaustive: match_directive");
	switch (directive.type) {
		// always matches, never applicable
		case ML_DT_ROOT:
			return true;
		case ML_DT_HEADER:
		case ML_DT_BLOCKQUOTE_HEADER:
		case ML_DT_HORIZONTAL_RULE:
			// singular line directive, we cannot match on future lines
			return false;
		case ML_DT_PARAGRAPH: {
			size_t offset = 0;
			size_t depth = count_whitespace(parser, &offset);
			if (depth != directive.depth) {
				return false;
			}
			if (at_whitespace(parser, offset)) {
				return false;
			}
			parser->allow_paragraph = false;
			if (advance_by)
				*advance_by = offset;
			return true;
		}
		case ML_DT_BLOCKQUOTE_BODY: {
			size_t offset;
			size_t length = 0;
			uint32_t bar   = peek_char(parser, 0, &offset, true);
			length += offset;
			uint32_t space = peek_char(parser, length, &offset, true);
			offset += length;
			if (bar != '|' || space != ' ') {
				return false;
			}
			if (advance_by)
				*advance_by = offset;
			return true;
		}
		case ML_DT_ORDERED_LIST_ITEM: {
			size_t offset;
			size_t depth = count_whitespace_limit(parser, &offset, directive.depth);
			if (depth != directive.depth) {
				parser->must_match_new_list_item_or_list_ends = LI_ORDERED;
				return false;
			}
			if (advance_by)
				*advance_by = offset;
			return true;
		}
		case ML_DT_UNORDERED_LIST_ITEM: {
			size_t offset;
			size_t depth = count_whitespace_limit(parser, &offset, 2);
			if (depth != 2) {
				parser->must_match_new_list_item_or_list_ends = LI_UNORDERED;
				return false;
			}
			if (advance_by)
				*advance_by = offset;
			return true;
		}
		case ML_DT_COUNT:
			fprintf(stderr, "unreachable\n");
			exit(1);
	}
	assert(false && "TODO!");
}

static void add_child_component(markless_component* parent, markless_component* child) {
	_Static_assert(ML_CT_COUNT == 12, "non-exhaustive");
	switch (parent->type) {
		case ML_CT_ROOT_DOCUMENT: {
			vector_push(parent->root->children, child);
			return;
		}
		case ML_CT_HEADER: {
			if (child->type == ML_CT_NEWLINE) {
				free(child);
				return; // newlines are skipped
			}
			vector_push(parent->header->children, child);
			return;
		}
		case ML_CT_PARAGRAPH: {
			vector_push(parent->paragraph->children, child);
			return;
		}
		case ML_CT_BLOCKQUOTE_BODY: {
			vector_push(parent->blockquote_body->children, child);
			return;
		}
		case ML_CT_BLOCKQUOTE_HEADER: {
			if (child->type == ML_CT_NEWLINE) {
				free(child);
				return; // newlines are skipped
			}
			vector_push(parent->blockquote_header->children, child);
			return;
		}
		case ML_CT_ORDERED_LIST: {
			if (child->type != ML_CT_ORDERED_LIST_ITEM) {
				fprintf(stderr, "Cannot add non-ordered list item to ordered list\n");
				exit(1);
			}
			vector_push(parent->ordered_list->items, child);
			return;
		}
		case ML_CT_ORDERED_LIST_ITEM: {
			if (child->type == ML_CT_NEWLINE) {
				free(child);
				return; // newlines are skipped
			}
			vector_push(parent->ordered_list_item->children, child);
			return;
		}
		case ML_CT_UNORDERED_LIST: {
			if (child->type != ML_CT_UNORDERED_LIST_ITEM) {
				fprintf(stderr, "Cannot add non-unordered list item to ordered list\n");
				exit(1);
			}
			vector_push(parent->unordered_list->items, child);
			return;
		}
		case ML_CT_UNORDERED_LIST_ITEM: {
			if (child->type == ML_CT_NEWLINE) {
				free(child);
				return; // newlines are skipped
			}
			vector_push(parent->unordered_list_item->children, child);
			return;
		}
		case ML_CT_TEXT: {
			fprintf(stderr, "Cannot add child to text node\n");
			exit(1);
		}
		case ML_CT_NEWLINE: {
			fprintf(stderr, "Cannot add child to newline entity\n");
			exit(1);
		}
		case ML_CT_HORIZONTAL_RULE: {
			if (child->type == ML_CT_NEWLINE) {
				free(child);
				return; // newlines are skipped
			}
			fprintf(stderr, "Cannot add child to horizontal rule\n");
			exit(1);
		}
		case ML_CT_COUNT: {
			fprintf(stderr, "unreachable\n");
			exit(1);
		}
	}
	assert(false && "TODO!");
}

static void unwind(parser_state* parser, int depth);

static void interrupt_directive(parser_state* parser, int type) {
	for (int i = (int)vector_count(parser->stack) - 1; i >= 0; i--) {
		if (parser->stack[i].directive.type == type) {
			unwind(parser, i);
			return;
		}
	}
}

static void interrupt_paragraph(parser_state* parser) {
	parser->allow_paragraph = true;
	interrupt_directive(parser, ML_DT_PARAGRAPH);
}

static bool digit(uint32_t c) {
	return c >= '0' && c <= '9';
}

static bool invoke(parser_state* parser) {
	stack_entry entry;
	size_t advance_by;
	uint32_t peeked = peek_char(parser, 0, &advance_by, true);

	// let's try a bunch of directives!

	// line directives
	if (parser->blockquote_body_special_case || !parser->inline_only) {
		if (peeked == '|') {
			size_t offset = advance_by;
			uint32_t next = peek_char(parser, offset, &advance_by, true);
			offset += advance_by;
			if (next == ' ') {
				entry.directive = (markless_directive){
					.type = ML_DT_BLOCKQUOTE_BODY,
				};
				// blockquote body
				interrupt_paragraph(parser);

				if (parser->blockquote_body_special_case) {
					interrupt_directive(parser, ML_DT_BLOCKQUOTE_HEADER);
				}

				parser->cursor += offset;
				parser->inline_only = false;
				parser->blockquote_body_special_case = false;

				markless_blockquote_body* body = (markless_blockquote_body*)malloc(sizeof(markless_blockquote_body));
				memset(body, 0, sizeof(markless_blockquote_body));

				entry.component = (markless_component*)malloc(sizeof(markless_component));
				entry.component->type = ML_CT_BLOCKQUOTE_BODY;
				entry.component->blockquote_body = body;

				vector_push(parser->stack, entry);
				return true;
			}
			if (parser->inline_only) goto inline_only;
		}
	}
inline_only:
	if (!parser->inline_only) {
		if (digit(peeked)) {
			size_t offset = 0;
			int value = 0;
			uint32_t next;
			int depth = 0;
			for (next = peeked; digit(next); next = peek_char(parser, offset, &advance_by, true)) {
				value *= 10;
				value += (int)next - '0';
				offset += advance_by;
				depth++;
			}
			if (next == '.') {
				offset += advance_by;
				depth++;
				entry.directive = (markless_directive){
					.type = ML_DT_ORDERED_LIST_ITEM,
					.depth = (size_t)depth,
				};
				// ordered list
				interrupt_paragraph(parser);
				parser->cursor += offset;

				markless_ordered_list_item* list_item = (markless_ordered_list_item*)malloc(sizeof(markless_ordered_list_item));
				memset(list_item, 0, sizeof(markless_ordered_list_item));
				list_item->number = value;

				entry.component = (markless_component*)malloc(sizeof(markless_component));
				entry.component->type = ML_CT_ORDERED_LIST_ITEM;
				entry.component->ordered_list_item = list_item;

				vector_push(parser->stack, entry);
				if (parser->must_match_new_list_item_or_list_ends == LI_ORDERED) {
					parser->must_match_new_list_item_or_list_ends = LI_NONE;
				}
				return true;
			}
		} else if (peeked == '-') {
			size_t offset = advance_by;
			int value = 0;
			uint32_t next = peek_char(parser, offset, &advance_by, true);
			if (next == ' ') {
				offset += advance_by;
				entry.directive = (markless_directive){
					.type = ML_DT_UNORDERED_LIST_ITEM,
				};
				// unordered list
				interrupt_paragraph(parser);
				parser->cursor += offset;

				markless_unordered_list_item* list_item = (markless_unordered_list_item*)malloc(sizeof(markless_unordered_list_item));
				memset(list_item, 0, sizeof(markless_unordered_list_item));

				entry.component = (markless_component*)malloc(sizeof(markless_component));
				entry.component->type = ML_CT_UNORDERED_LIST_ITEM;
				entry.component->unordered_list_item = list_item;

				vector_push(parser->stack, entry);
				if (parser->must_match_new_list_item_or_list_ends == LI_UNORDERED) {
					parser->must_match_new_list_item_or_list_ends = LI_NONE;
				}
				return true;
			}
		} else if (peeked == '~') {
			size_t offset = advance_by;
			uint32_t next = peek_char(parser, offset, &advance_by, true);
			offset += advance_by;
			if (next == ' ') {
				entry.directive = (markless_directive){
					.type = ML_DT_BLOCKQUOTE_HEADER,
				};
				// blockquote header
				interrupt_paragraph(parser);
				parser->cursor += offset;
				parser->inline_only = true;
				parser->blockquote_body_special_case = true;

				markless_blockquote_header* header = (markless_blockquote_header*)malloc(sizeof(markless_blockquote_header));
				memset(header, 0, sizeof(markless_blockquote_header));

				entry.component = (markless_component*)malloc(sizeof(markless_component));
				entry.component->type = ML_CT_BLOCKQUOTE_HEADER;
				entry.component->blockquote_header = header;

				vector_push(parser->stack, entry);
				return true;
			}
		} else if (peeked == '#') {
			// it's headering time?
			int level = 1;
			size_t offset = advance_by;
			while (peek_char(parser, offset, &advance_by, true) == '#') {
				level++;
				offset += advance_by;
			}
			if (peek_char(parser, offset, &advance_by, true) != ' ') {
				return false;
			}
			offset += advance_by;
			// headering time
			interrupt_paragraph(parser);
			entry.directive = (markless_directive){
				.type  = ML_DT_HEADER,
			};

			parser->cursor += offset;
			parser->inline_only = true;

			markless_header* h = (markless_header*)malloc(sizeof(markless_header));
			memset(h, 0, sizeof(markless_header));

			h->level = level;

			entry.component = (markless_component*)malloc(sizeof(markless_component));
			entry.component->type = ML_CT_HEADER;
			entry.component->header = h;

			vector_push(parser->stack, entry);
			return true;
		} else if (peeked == '=') {
			// horizontal rule, maybe
			int count = 1;
			size_t offset = advance_by;
			while (peek_char(parser, offset, &advance_by, true) == '=') {
				count++;
				offset += advance_by;
			}
			if (peek_char(parser, offset, &advance_by, true) != '\n') {
				return false;
			}
			// horizontal rule
			interrupt_paragraph(parser);
			entry.directive = (markless_directive){
				.type  = ML_DT_HORIZONTAL_RULE,
			};

			parser->cursor += offset;

			entry.component = (markless_component*)malloc(sizeof(markless_component));
			entry.component->type = ML_CT_HORIZONTAL_RULE;

			vector_push(parser->stack, entry);
			return true;
		}
	}
	// inline directives
	// none yet, let's see abt paragraph
	if (parser->allow_paragraph && !parser->inline_only) {
		// paragraph tiem
		// let's consume all the preceding whitespace
		size_t depth = count_whitespace(parser, &advance_by);

		if (at_whitespace(parser, advance_by)) {
			// not a paragraph, bail
			return false;
		}

		parser->cursor += advance_by;

		parser->allow_paragraph = false;

		entry.directive = (markless_directive){
			.type  = ML_DT_PARAGRAPH,
			.depth = depth,
		};

		markless_paragraph* p = (markless_paragraph*)malloc(sizeof(markless_paragraph));
		memset(p, 0, sizeof(markless_paragraph));

		entry.component = (markless_component*)malloc(sizeof(markless_component));
		entry.component->type = ML_CT_PARAGRAPH;
		entry.component->paragraph = p;

		vector_push(parser->stack, entry);
		return true;
	}

	return false;
}

static void cleanup(parser_state* parser, stack_entry disposing) {
	_Static_assert(ML_DT_COUNT == 8, "non-exhaustive: cleanup");
	switch (disposing.directive.type) {
		case ML_DT_ROOT:
			fprintf(stderr, "WE SHOULD NOT BE DISPOSING ROOT\n");
			exit(1);
		case ML_DT_HEADER:
		case ML_DT_PARAGRAPH:
		case ML_DT_BLOCKQUOTE_BODY:
		case ML_DT_BLOCKQUOTE_HEADER:
		case ML_DT_HORIZONTAL_RULE: // trivial
			{
				size_t idx = (size_t)vector_count(parser->stack) - 2;
				add_child_component(parser->stack[idx].component, disposing.component);
			}
			return;
		case ML_DT_ORDERED_LIST_ITEM:
			{
				size_t idx = (size_t)vector_count(parser->stack) - 2;
				markless_component* parent = parser->stack[idx].component;
				markless_component* last_child = get_last_child(parent);
				if (last_child && last_child->type == ML_CT_ORDERED_LIST && last_child->ordered_list->active) {
					add_child_component(last_child, disposing.component);
				} else {
					markless_ordered_list* ol = (markless_ordered_list*)malloc(sizeof(markless_ordered_list));
					memset(ol, 0, sizeof(markless_ordered_list));
					ol->active = true;

					markless_component* component = (markless_component*)malloc(sizeof(markless_component));
					component->type = ML_CT_ORDERED_LIST;
					component->ordered_list = ol;
					add_child_component(parent, component);
					add_child_component(component, disposing.component);
				}
			}
			return;
		case ML_DT_UNORDERED_LIST_ITEM:
			{
				size_t idx = (size_t)vector_count(parser->stack) - 2;
				markless_component* parent = parser->stack[idx].component;
				markless_component* last_child = get_last_child(parent);
				if (last_child && last_child->type == ML_CT_UNORDERED_LIST && last_child->unordered_list->active) {
					add_child_component(last_child, disposing.component);
				} else {
					markless_unordered_list* ul = (markless_unordered_list*)malloc(sizeof(markless_unordered_list));
					memset(ul, 0, sizeof(markless_unordered_list));
					ul->active = true;

					markless_component* component = (markless_component*)malloc(sizeof(markless_component));
					component->type = ML_CT_UNORDERED_LIST;
					component->unordered_list = ul;
					add_child_component(parent, component);
					add_child_component(component, disposing.component);
				}
			}
			return;
		case ML_DT_COUNT:
			fprintf(stderr, "unreachable\n");
			exit(1);
	}
	assert(false && "TODO!");
}

static void unwind(parser_state* parser, int depth) {
	while (vector_count(parser->stack) > depth) {
		size_t idx = (size_t)vector_count(parser->stack) - 1;
		stack_entry disposing = parser->stack[idx];
		cleanup(parser, disposing);
		vector_pop(parser->stack);
	}
}

markless_doc* parse_markless_document(sitegen_context* context, sourcebuffer source) {
	markless_doc* document = (markless_doc*)malloc(sizeof(markless_doc));
	parser_state* parser   = (parser_state*)malloc(sizeof(parser_state));
	memset(document, 0, sizeof(markless_doc));
	memset(parser,   0, sizeof(parser_state));

	parser->source = source;
	parser->linebreak_mode = LB_MODE_SHOW;

	markless_component root_component;
	root_component.type = ML_CT_ROOT_DOCUMENT;
	root_component.root = document;

	stack_entry root_entry = (stack_entry){
		.directive = (markless_directive){
			.type = ML_DT_ROOT,
		},
		.component = &root_component,
	};

	vector_push(parser->stack, root_entry);

	while (parser->cursor < parser->source.data.len) {
		int current_entry;
		parser->allow_paragraph = true;
		parser->inline_only = false;
		parser->blockquote_body_special_case = false;
		parser->must_match_new_list_item_or_list_ends = LI_NONE;
		for (current_entry = 0; current_entry < vector_count(parser->stack); current_entry++) {
			size_t advance_by = 0;
			if (match_directive(parser, parser->stack[current_entry].directive, &advance_by)) {
				parser->cursor += advance_by;
				continue; // all good
			} else {
				// stack unwind
				unwind(parser, current_entry);
				break;
			}
		}

		//printf("non-invoke matched parsing: ");
		while (!at_end_of_line(parser)) {
			// invoke
			if (!invoke(parser)) {
				parser->inline_only = true; // we passed the start of the line
				size_t advance_by;
				uint32_t c = peek_char(parser, 0, &advance_by, true);
				if (c == 0) goto end;
				if (c != ('\n' | 0x80000000)) {
					add_char_to_component(parser->stack[vector_count(parser->stack) - 1].component, c);
				}
				parser->cursor += advance_by;
			}
		}
		size_t advance_by;
		uint32_t c = peek_char(parser, 0, &advance_by, true);
		if (c == '\n') {
			// handle newline stuff here
			if (parser->linebreak_mode == LB_MODE_SHOW && vector_count(parser->stack) > 1) {
				markless_component* component = (markless_component*)malloc(sizeof(markless_component));
				component->type = ML_CT_NEWLINE;
				add_child_component(parser->stack[vector_count(parser->stack) - 1].component, component);
			}
			parser->cursor += advance_by;
		}
		if (parser->must_match_new_list_item_or_list_ends == LI_ORDERED) {
			markless_component* parent = vector_back(parser->stack)->component;
			markless_component* last_child = get_last_child(parent);
			if (last_child->type == ML_CT_ORDERED_LIST && last_child->ordered_list->active) {
				last_child->ordered_list->active = false;
			}
		}
		else if (parser->must_match_new_list_item_or_list_ends == LI_UNORDERED) {
			markless_component* parent = vector_back(parser->stack)->component;
			markless_component* last_child = get_last_child(parent);
			if (last_child->type == ML_CT_UNORDERED_LIST && last_child->unordered_list->active) {
				last_child->unordered_list->active = false;
			}
		}
		//printf("\n");

		//assert(false && "TODO!");
	}
end:
	// stack unwind
	unwind(parser, 1);

	vector_free(parser->stack);
	vector_free(parser->disabled_directives);
	free(parser);

	return document;
}
