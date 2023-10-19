#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>

#include "sitegen/document_tree.h"
#include "sitegen/stringview.h"

typedef struct {
	stringview path;
	int line;
	int column;
	int index;
} location;

static uint32_t read_utf8_followers(int count, location* c, stringview string) {
	if (c->index + count > string.len) {
		fprintf(stderr, "not enough bytes in string");
		exit(1);
	}

	uint32_t value = 0;
	for (int i = 0; i < count; i++) {
		value <<= 6;
		uint8_t follower = (uint8_t)string.data[c->index + i];
		if ((follower & 0xC0) != 0x80) {
			fprintf(stderr, "Invalid UTF-8 Sequence: %02X\n", follower);
			exit(1);
		}

		value |= follower & 0x3F;
	}

	c->index += count;

	return value;
}

static uint32_t read_codepoint(location* c, stringview string) {
	if (c->index == string.len) {
		return 0; // end of string
	}
	// are we dealing with a utf-8 header byte rn?
	uint8_t initial = (uint8_t)string.data[c->index];
	c->index++;
	if ((initial & 0x80) == 0) {
		// just ascii
		if (initial == '\r' || initial == '\n') c->column = 0; // reset column
		if (initial == '\n') c->line++; // increment line
		return initial; // so we just return
	} else if ((initial & 0xE0) == 0xC0) {
		// 2 byte utf-8
		return read_utf8_followers(1, c, string) | (initial & 0x1F) << 6;
	} else if ((initial & 0xF0) == 0xE0) {
		// 3 byte utf-8
		return read_utf8_followers(2, c, string) | (initial & 0x0F) << 12;
	} else if ((initial & 0xF8) == 0xF0) {
		// 4 byte utf-8
		return read_utf8_followers(3, c, string) | (initial & 0x07) << 18;
	} else {
		fprintf(stderr, "Invalid UTF-8 Sequence\n");
		exit(1);
	}
}

static uint32_t peek_codepoint(location* c, stringview string) {
	location copy = *c;
	uint32_t value = read_codepoint(c, string);
	*c = copy;
	return value;
}

static docnode* parse_node(sourcebuffer source, location* c, bool breakOnNewline) {
	docnode* current = malloc(sizeof(docnode));

	uint32_t codepoint = peek_codepoint(c, source.data);
	if (codepoint == 0) return NULL;

	vector(char) plaintext_value = NULL;

	while (codepoint) {
		printf("Current char: %lc\n", codepoint);
		switch (codepoint) {
			case '#': {
				if (plaintext_value != NULL) goto plaintext_return;
				docnode_header header;
				header.level = 0;
				while (codepoint == '#') {
					read_codepoint(c, source.data);
					header.level++;
					codepoint = peek_codepoint(c, source.data);
				}
				if (codepoint != ' ') {
					fprintf(stderr, "Expected space after `#`\n");
					exit(1);
				}
				read_codepoint(c, source.data);
				printf("level: %d\n", header.level);
				docnode* child = NULL;
				while ((child = parse_node(source, c, true))) {
					vector_push(header.subnodes, *child);
				}
				current->kind == NK_HEADER;
				current->value.header = header;
				return current;
			} break;
		}
	}

	assert(false && "TODO!");

	return current;
}

document* parse_document_tree(sourcebuffer source) {
	location c;
	c.path = source.path;
	c.line = 0;
	c.column = 0;
	c.index = 0;

	document* doc = (document*)malloc(sizeof(document));

	docnode* current = NULL;
	while ((current = parse_node(source, &c, false))) {
		vector_push(doc->nodes, *current);
	}

	/*
	*/

	printf("Data:\n"STRINGVIEW_FMT"\nc:\n", STRINGVIEW_SPILL(source.data));
	assert(false && "TODO!");

	return doc;
}
