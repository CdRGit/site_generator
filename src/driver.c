#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>

#include "sitegen/site_generator.h"

const char* usage_format = "Usage: %s [options] file...\n";

static bool parse_args(sitegen_context* context, int argc, char** argv);
static void arg_shift(int* argc, char*** argv);

int main(int argc, char** argv) {
	char* l = setlocale(LC_ALL, "");
	sitegen_context* context = sitegen_context_create();
	assert(context);

	char* program_name = *argv;
	arg_shift(&argc, &argv);

	if (!parse_args(context, argc, argv) || context->help || 0 == vector_count(context->files)) {
		printf(usage_format, program_name);
		sitegen_context_destroy(context);
		return 0;
	}

	sitegen_generate(context);

	sitegen_context_destroy(context);
	return 0;
}

static bool parse_args(sitegen_context* context, int argc, char** argv) {
	if (argc == 0) return true;

	if (false) {
	} else {
		vector_push(context->files, stringview_create(argv[0], strlen(argv[0])));
	}

	arg_shift(&argc, &argv);
	return parse_args(context, argc, argv);
}

static void arg_shift(int* argc, char*** argv) {
	(*argc)--;
	(*argv)++;
}
