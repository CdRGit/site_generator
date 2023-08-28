#include "sitegen/stringview.h"

stringview stringview_create(char* data, size_t len) {
	return (stringview){
		.data = data,
		.len = len,
	};
}
