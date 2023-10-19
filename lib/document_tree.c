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

document* parse_document_tree(sourcebuffer source) {
	vector(uint32_t) doc_as_codepoints;
	int index = 0;

	uint32_t codepoint = 0;
	while ((codepoint = read_codepoint(&index, source.data))) {
		vector_push(doc_as_codepoints, codepoint);
	}

	printf("Data:\n");
	for (int i = 0; i < vector_count(doc_as_codepoints); i++) {
		printf("%lc", doc_as_codepoints[i]);
	}
	printf("\n");
	assert(false && "TODO!");
}
