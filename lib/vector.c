// See bottom of file for copyright info
#include "sitegen/vector.h"

void vector_maybe_expand(void** vector_ref, long long element_size, long long required_count)
{
	struct vector_header* header = vector_get_header(*vector_ref);
	if (!*vector_ref)
	{
		long long initial_capacity = 32;
		void* new_data = malloc((sizeof *header) + (unsigned long long)(initial_capacity * element_size));
		header = new_data;

		header->capacity = initial_capacity;
		header->count = 0;

	}
	else if (required_count >= header->capacity)
	{
		while (required_count >= header->capacity)
			header->capacity *= 2;
		header = realloc(header, (sizeof *header) + (unsigned long long)(header->capacity * element_size));
	}

	*vector_ref = (void*)(header + 1);
}

// MIT License
// Copyright (c) 2023 Local C. Atticus
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
