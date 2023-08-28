// See bottom of file for copyright info
#ifndef SITEGEN_VECTOR_H
#define SITEGEN_VECTOR_H

#include <stdlib.h>

struct vector_header {
	long long capacity;
	long long count;
};

void vector_maybe_expand(void** vector_ref, long long element_size, long long required_count);

#define vector(T) T*
#define vector_get_header(V) ((struct vector_header*)((V) - 1))
#define vector_count(V) ((V) ? vector_get_header(V)->count : 0)
#define vector_push(V, E) do { vector_maybe_expand((void**)&(V), (long long)sizeof *(V), vector_count(V) + 1); (V)[vector_count(V)] = E; vector_get_header(V)->count++; } while (0)
#define vector_pop(V) do { if (vector_get_header(V)->count) vector_get_header(V)->count--; } while (0)
#define vector_back(V) (&(V)[vector_count(V) - 1])
#define vector_free(V) do { if (V) { free(vector_get_header(V)); (V) = NULL; } } while (0)

#endif//SITEGEN_VECTOR_H

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
