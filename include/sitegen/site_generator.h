#ifndef SITEGEN_SITE_GENERATOR_H
#define SITEGEN_SITE_GENERATOR_H

#include <stdbool.h>

#include "sitegen/vector.h"
#include "sitegen/stringview.h"

typedef struct {
	bool help;
	vector(stringview) files;
} sitegen_settings;

sitegen_settings* sitegen_settings_create(void);

void sitegen_settings_destroy(sitegen_settings* settings);

#endif//SITEGEN_SITE_GENERATOR_H
