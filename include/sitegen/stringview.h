#ifndef SITEGEN_STRINGVIEW_H
#define SITEGEN_STRINGVIEW_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	char* data;
	size_t len;
} stringview;

stringview stringview_create(char* data, size_t len);

bool stringview_equal(stringview left, stringview right);

typedef struct {
	uint32_t* data;
	size_t len;
} stringview_u32;

stringview_u32 stringview_u32_create(uint32_t* data, size_t len);

bool stringview_u32_equal(stringview_u32 left, stringview_u32 right);

#define STRINGVIEW_SPILL(sv) (int)(sv.len), (sv.data)
#define STRINGVIEW_FMT "%.*ls"
#define STRINGVIEW_U32_FMT "%.*s"

#endif//SITEGEN_STRINGVIEW_H
