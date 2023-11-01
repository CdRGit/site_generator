#ifndef MARKLESS_DOCUMENT_H
#define MARKLESS_DOCUMENT_H

#include "sitegen/stringview.h"

typedef struct {
	struct {
		stringview author;
		stringview copyright;
		stringview language;
	} metadata;
} markless_doc;

typedef struct {
	enum {
		ML_CT_ROOT_DOCUMENT
	} type;
	union {
		markless_doc* root;
	};
} markless_component;

#endif//MARKLESS_DOCUMENT_H
