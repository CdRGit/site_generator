#ifndef SITEGEN_STRINGVIEW_H
#define SITEGEN_STRINGVIEW_H

#include <stddef.h>

typedef struct {
	char* data;
	size_t len;
} stringview;

stringview stringview_create(char* data, size_t len);

#endif//SITEGEN_STRINGVIEW_H
