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

static uint32_t read_utf8_followers(int count, int* index, stringview string) {
	if (*index + count > string.len) {
		fprintf(stderr, "not enough bytes in string");
		exit(1);
	}

	uint32_t value = 0;
	for (int i = 0; i < count; i++) {
		value <<= 6;
		uint8_t follower = (uint8_t)string.data[*index + i];
		if ((follower & 0xC0) != 0x80) {
			fprintf(stderr, "Invalid UTF-8 Sequence: %02X\n", follower);
			exit(1);
		}

		value |= follower & 0x3F;
	}

	(*index) += count;

	return value;
}

static uint32_t read_codepoint(int* index, stringview string) {
	if (*index == string.len) {
		return 0; // end of string
	}
	// are we dealing with a utf-8 header byte rn?
	uint8_t initial = (uint8_t)string.data[*index];
	(*index)++;
	if ((initial & 0x80) == 0) {
		// just ascii
		return initial; // so we just return
	} else if ((initial & 0xE0) == 0xC0) {
		// 2 byte utf-8
		return read_utf8_followers(1, index, string) | (initial & 0x1F) << 6;
	} else if ((initial & 0xF0) == 0xE0) {
		// 3 byte utf-8
		return read_utf8_followers(2, index, string) | (initial & 0x0F) << 12;
	} else if ((initial & 0xF8) == 0xF0) {
		// 4 byte utf-8
		return read_utf8_followers(3, index, string) | (initial & 0x07) << 18;
	} else {
		fprintf(stderr, "Invalid UTF-8 Sequence\n");
		exit(1);
	}
}

static docnode* parse_node_pass_1(vector(uint32_t) codepoints, size_t* index) {
	docnode* node = (docnode*)malloc(sizeof(docnode));

	uint32_t currently_active_char  = 0;
	int currently_active_char_count = 0;
	bool has_only_been_that = true;
	int indent_depth = 0;
	bool done_indenting = false;

	size_t start = *index;
	size_t start_of_line = *index;

	while (*index < vector_count(codepoints)) {
		uint32_t point = codepoints[(*index)++];
		printf("char: %lc\n", point);
		if (point == ' ') {
			if (!done_indenting) indent_depth++;
		} else if (point == '\t') {
			if (!done_indenting) indent_depth += (4 - indent_depth % 4);
		} else if (point == '\r' || point == '\n') {
			uint32_t lookahead = codepoints[*index];
			if (lookahead == '\n' && point != '\n') (*index)++; // consume '\n' in '\r\n' to ensure line ending
			
			if (currently_active_char_count == 0) {
				// blank line!
				printf("BLANK LINE :D\n");
			} else {
				printf("non-blank line :c\n");

				if (has_only_been_that) {
					printf("\tsingle char tho :D : %lc x %d\n", currently_active_char, currently_active_char_count);
					switch (currently_active_char) {
						case '*':
						case '-':
						case '_': {
								if (currently_active_char_count >= 3) {
									printf("\t THEMATIC BREAK LETS GO\n");
									node->kind = NK_THEMATIC_BREAK;
									node->value.thematic_break.kind = currently_active_char;
									return node;
								}
							}
							break;
					}
				}
				else {
					// ATX heading?
					if (currently_active_char == '#') {
						// ATX HEADING?! :D
						// scan from start of line to now
					}
				}
			}
			currently_active_char = 0;
			currently_active_char_count = 0;
			has_only_been_that = true;
			indent_depth = 0;
			done_indenting = false;
			start_of_line = *index;
		} else if (currently_active_char_count == 0 || currently_active_char == point) {
			done_indenting = true;
			currently_active_char = point;
			currently_active_char_count++;
		} else {
			has_only_been_that = false;
		}
	}
	return node;
}

static document* parse_tree_pass_1(vector(uint32_t) codepoints) {
	size_t index = 0;
	document* pass1 = (document*)malloc(sizeof(document));
	pass1->nodes = NULL;

	while (index < vector_count(codepoints)) {
		docnode* node = parse_node_pass_1(codepoints, &index);
		vector_push(pass1->nodes, *node);
		free(node);
	}

	assert(false && "TODO!");

	return pass1;
}

document* parse_document_tree(sourcebuffer source) {
	vector(uint32_t) doc_as_codepoints;
	int index = 0;

	uint32_t codepoint = 0;
	while ((codepoint = read_codepoint(&index, source.data))) {
		vector_push(doc_as_codepoints, codepoint ? codepoint : 0xFFFD);
	}
	vector_push(doc_as_codepoints, 0);

	document* pass1 = parse_tree_pass_1(doc_as_codepoints);

	printf("Data:\n");
	for (int i = 0; i < vector_count(doc_as_codepoints); i++) {
		printf("%lc", doc_as_codepoints[i]);
	}
	printf("\n");
	assert(false && "TODO!");
}
