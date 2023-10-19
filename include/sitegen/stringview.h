#ifndef SITEGEN_STRINGVIEW_H
#define SITEGEN_STRINGVIEW_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
	char* data;
	size_t len;
} stringview;

stringview stringview_create(char* data, size_t len);

bool stringview_equal(stringview left, stringview right);

#define STRINGVIEW_SPILL(sv) (int)(sv.len), (sv.data)
#define STRINGVIEW_FMT "%.*s"

#endif//SITEGEN_STRINGVIEW_H
