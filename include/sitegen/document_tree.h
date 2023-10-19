#ifndef SITEGEN_DOCUMENT_TREE_H
#define SITEGEN_DOCUMENT_TREE_H

#include "sitegen/site_generator.h"

#include "sitegen/vector.h"
#include "sitegen/stringview.h"

typedef enum {
	NK_HEADER,
	NK_PARAGRAPH,
	NK_PLAINTEXT,
	NK_DECORATION,
	NK_LINEBREAK,
	NK_COUNT,
} docnode_kind;

typedef struct docnode docnode;

typedef struct {
} docnode_trivial;

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
	} decoration;
	vector(docnode) subnodes;
} docnode_decoration;

struct docnode {
	docnode_kind kind;
	_Static_assert(NK_COUNT == 5, "non-exhaustive");
	union {
		docnode_header      header;
		docnode_paragraph   paragraph;
		docnode_plaintext   plaintext;
		docnode_decoration  decoration;
		docnode_trivial     linebreak;
	} value;
};

typedef struct {
	vector(docnode) nodes;
} document;

document* parse_document_tree(sourcebuffer source);

#endif//SITEGEN_DOCUMENT_TREE_H
