#ifndef MARKLESS_DOCUMENT_H
#define MARKLESS_DOCUMENT_H

#include "sitegen/vector.h"
#include "sitegen/stringview.h"

typedef struct markless_component markless_component;

typedef struct {
	struct {
		stringview author;
		stringview copyright;
		stringview language;
	} metadata;

	vector(markless_component*) children;
} markless_doc;

typedef struct {
	vector(markless_component*) children;
	int level;
} markless_header;

typedef struct {
	vector(markless_component*) children;
} markless_paragraph;

typedef struct {
	vector(markless_component*) children;
} markless_blockquote_body;

typedef struct {
	vector(markless_component*) children;
} markless_blockquote_header;

typedef struct {
	vector(char) text;
} markless_text;

struct markless_component {
	enum {
		ML_CT_ROOT_DOCUMENT,
		ML_CT_HEADER,
		ML_CT_PARAGRAPH,
		ML_CT_BLOCKQUOTE_BODY,
		ML_CT_BLOCKQUOTE_HEADER,
		ML_CT_TEXT,
		ML_CT_NEWLINE,
		ML_CT_COUNT,
	} type;
	_Static_assert(ML_CT_COUNT == 7, "non-exhaustive");
	union {
		markless_doc*               root;
		markless_header*            header;
		markless_paragraph*         paragraph;
		markless_blockquote_body*   blockquote_body;
		markless_blockquote_header* blockquote_header;
		markless_text*              text;
		// newline
	};
};

void free_document(markless_doc* document);

void print_document(markless_doc* document);

#endif//MARKLESS_DOCUMENT_H
