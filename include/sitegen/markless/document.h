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
	vector(markless_component*) children;
	int number;
} markless_ordered_list_item;

typedef struct {
	vector(markless_component*) items;
	bool active;
} markless_ordered_list;

typedef struct {
	vector(markless_component*) children;
} markless_unordered_list_item;

typedef struct {
	vector(markless_component*) items;
	bool active;
} markless_unordered_list;

typedef struct {
	vector(char) language;
	vector(char) options;
	vector(char) text;
} markless_codeblock;

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
		ML_CT_ORDERED_LIST,
		ML_CT_ORDERED_LIST_ITEM,
		ML_CT_UNORDERED_LIST,
		ML_CT_UNORDERED_LIST_ITEM,
		ML_CT_CODEBLOCK,
		ML_CT_TEXT,
		ML_CT_NEWLINE,
		ML_CT_HORIZONTAL_RULE,
		ML_CT_COUNT,
	} type;
	_Static_assert(ML_CT_COUNT == 13, "non-exhaustive");
	union {
		markless_doc*                 root;
		markless_header*              header;
		markless_paragraph*           paragraph;
		markless_blockquote_body*     blockquote_body;
		markless_blockquote_header*   blockquote_header;
		markless_ordered_list*        ordered_list;
		markless_ordered_list_item*   ordered_list_item;
		markless_unordered_list*      unordered_list;
		markless_unordered_list_item* unordered_list_item;
		markless_codeblock*           codeblock;
		markless_text*                text;
		// newline
		// horizontal_rule
	};
};

void free_document(markless_doc* document);

void print_document(markless_doc* document);

#endif//MARKLESS_DOCUMENT_H
