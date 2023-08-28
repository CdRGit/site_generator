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
