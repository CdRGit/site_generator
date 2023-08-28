#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "sitegen/site_generator.h"

const char* usage_format = "Usage: %s [options] file...\n";

static bool parse_args(sitegen_settings* settings, int argc, char** argv);
static void arg_shift(int* argc, char*** argv);

int main(int argc, char** argv) {
	sitegen_settings* settings = sitegen_settings_create();
	assert(settings);

	char* program_name = *argv;
	arg_shift(&argc, &argv);

	if (!parse_args(settings, argc, argv) || settings->help) {
		printf(usage_format, program_name);
		sitegen_settings_destroy(settings);
		return 0;
	}

	sitegen_settings_destroy(settings);
	return 0;
}

static bool parse_args(sitegen_settings* settings, int argc, char** argv) {
	if (argc == 0) return true;

	if (false) {
	} else {
		vector_push(settings->files, stringview_create(argv[0], strlen(argv[0])));
	}

	arg_shift(&argc, &argv);
	return parse_args(settings, argc, argv);
}

static void arg_shift(int* argc, char*** argv) {
	(*argc)--;
	(*argv)++;
}
