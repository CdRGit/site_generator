#include <string.h>

#include "sitegen/stringview.h"

stringview stringview_create(char* data, size_t len) {
	return (stringview){
		.data = data,
		.len = len,
	};
}

bool stringview_equal(stringview left, stringview right) {
	return (0 == strncmp(left.data, right.data, left.len));
}

stringview_u32 stringview_u32_create(uint32_t* data, size_t len) {
	return (stringview_u32){
		.data = data,
		.len = len,
	};
}

bool stringview_u32_equal(stringview_u32 left, stringview_u32 right) {
	return (0 == memcmp(left.data, right.data, left.len * 4));
}
