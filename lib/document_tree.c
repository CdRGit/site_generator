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

typedef struct {
	uint32_t* data;
	size_t len;
} u32_stringview;

static u32_stringview trim_whitespace(u32_stringview view) {
	u32_stringview trimmed = view;

	size_t start_idx = 0;

	for (;;) {
		uint32_t point = view.data[start_idx];
		switch (point) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				start_idx++;
				break;
			default:
				goto start_done;
		}
	}
start_done:
	trimmed.data += start_idx;
	trimmed.len  -= start_idx;

	size_t end_idx = trimmed.len - 1;
	for (;;) {
		uint32_t point = trimmed.data[end_idx];
		switch (point) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				end_idx--;
				break;
			default:
				goto end_done;
		}
	}
end_done:
	trimmed.len = end_idx + 1;

	return trimmed;
}

static docnode* parse_node_pass_1(vector(uint32_t) codepoints, size_t* index) {
	docnode* node = (docnode*)malloc(sizeof(docnode));

	size_t end_index = *index;
	// find end of line
	while (codepoints[end_index] != 0) {
		uint32_t point = codepoints[end_index++];
		if (point == '\r') {
			uint32_t newline_maybe = codepoints[end_index];
			if (newline_maybe == '\n') {
				end_index++;
			}
			break;
		} else if (point == '\n') {
			break;
		}
	}
	u32_stringview line;
	line.data = codepoints + *index;
	line.len = end_index - *index;
	u32_stringview trimmed = trim_whitespace(line);
	printf("line: \"%.*ls\"\n", STRINGVIEW_SPILL(trimmed));

	uint32_t character = trimmed.data[0];
	size_t length = 0;

	while (trimmed.data[length] == character) length++;

	*index = end_index;

	if (length == trimmed.len) {
		if (length == 0) {
			assert(false && "TODO! all-whitespace");
		}
		// all-one-character
		if (length >= 3) {
			switch (character) {
				case '_':
				case '-':
				case '*':
					printf("    NK_THEMATIC_BREAK\n");
					node->kind = NK_THEMATIC_BREAK;
					node->value.thematic_break.kind = character;
					return node;
			}
		}
	}

	assert(false && "TODO!");

	return node;
}

static document* parse_tree_pass_1(vector(uint32_t) codepoints) {
	size_t index = 0;
	document* pass1 = (document*)malloc(sizeof(document));
	pass1->nodes = NULL;

	while (codepoints[index] != 0) {
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
