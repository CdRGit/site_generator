#include <stdlib.h>

#include "sitegen/site_generator.h"

sitegen_settings* sitegen_settings_create(void) {
	sitegen_settings* settings = calloc(1, sizeof *settings);
	return settings;
}

void sitegen_settings_destroy(sitegen_settings* settings) {
	free(settings);
}
