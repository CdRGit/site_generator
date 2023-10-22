#ifndef SITEGEN_DOCUMENT_TREE_H
#define SITEGEN_DOCUMENT_TREE_H
#include <stdint.h>

#include "sitegen/site_generator.h"

#include "sitegen/vector.h"
#include "sitegen/stringview.h"

typedef enum {
	NK_THEMATIC_BREAK,
	NK_TEMPORARY_TEXT,
	NK_COUNT,
} docnode_kind;

typedef struct docnode docnode;

// during construction
typedef struct {
	uint32_t* data;
	size_t  length;
} docnode_temporary_text;

// leaf blocks
typedef struct {
	uint32_t kind;
} docnode_thematic_break;

typedef struct {
} docnode_trivial;

// others
typedef struct {
	int level;
	vector(docnode) subnodes;
} docnode_header;

typedef struct {
	vector(docnode) subnodes;
} docnode_paragraph;

typedef struct {
	stringview text;
} docnode_plaintext;

typedef struct {
	enum {
		DEC_BOLD,
		DEC_ITALICS,
		DEC_UNDERLINE,
		DEC_STRIKETHROUGH,
		DEC_SUPERSCRIPT,
		DEC_SUBSCRIPT,
		DEC_COUNT,
	} decoration;
	vector(docnode) subnodes;
} docnode_decoration;

struct docnode {
	docnode_kind kind;
	_Static_assert(NK_COUNT == 2, "non-exhaustive");
	union {
		// leaf blocks
		docnode_thematic_break thematic_break;

		docnode_temporary_text temp_text;
	} value;
};

typedef struct {
	vector(docnode) nodes;
} document;

document* parse_document_tree(sourcebuffer source);

#endif//SITEGEN_DOCUMENT_TREE_H
