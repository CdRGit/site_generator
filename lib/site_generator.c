#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "sitegen/site_generator.h"
#include "sitegen/document_tree.h"

sitegen_context* sitegen_context_create(void) {
	sitegen_context* context = calloc(1, sizeof *context);
	return context;
}

void sitegen_context_destroy(sitegen_context* context) {
	free(context);
}

void sitegen_generate(sitegen_context* context) {
	for (int i = 0; i < vector_count(context->files); i++) {
		sitegen_load_buffer(context, context->files[i]);
		document* tree = parse_document_tree(context->buffers[i]);



		free(tree);
	}
}

static stringview load_stringview_from_file(char* path_cstr) {
	FILE* f;
	char* buffer = NULL;
	size_t length;

	f = fopen(path_cstr, "r");
	if (f) {
		fseek(f, 0, SEEK_END);
		length = (size_t)ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length + 1);
		if (buffer)
		{
			fread(buffer, 1, length, f);
		}
		fclose(f);
	} else {
		fprintf(stderr, "File %s not found\n", path_cstr);
		exit(1);
	}

	if (!buffer) {
		fprintf(stderr, "issue creating buffer\n");
		exit(1);
	}

	return (stringview) {
		.data = buffer,
		.len = length,
	};
}

void sitegen_load_buffer(sitegen_context* context, stringview path) {
	sourcebuffer buffer;
	for (int i = 0; i < vector_count(context->buffers); i++) {
		buffer = context->buffers[i];

		if (stringview_equal(buffer.path, path)) return; // early out
	}

	char* path_cstr = (char*)malloc(path.len + 1);
	memcpy(path_cstr, path.data, path.len);
	path_cstr[path.len] = '\0';

	buffer = (sourcebuffer){.path = path, .data = load_stringview_from_file(path_cstr)};

	vector_push(context->buffers, buffer);

	free(path_cstr);
}
