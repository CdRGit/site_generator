#ifndef SITEGEN_SITE_GENERATOR_H
#define SITEGEN_SITE_GENERATOR_H

#include <stdbool.h>

#include "sitegen/vector.h"
#include "sitegen/stringview.h"

typedef struct {
	stringview path;
	stringview data;
} sourcebuffer;

typedef struct {
	bool help;
	vector(stringview) files;
	vector(sourcebuffer) buffers;
} sitegen_context;

sitegen_context* sitegen_context_create(void);

void sitegen_context_destroy(sitegen_context* context);

void sitegen_generate(sitegen_context* context);

void sitegen_load_buffer(sitegen_context* context, stringview path);

#endif//SITEGEN_SITE_GENERATOR_H
