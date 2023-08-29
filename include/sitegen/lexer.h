#ifndef SITEGEN_LEXER_H
#define SITEGEN_LEXER_H

#include "sitegen/vector.h"
#include "sitegen/stringview.h"

typedef enum {
	TK_MULTIBYTE = 255, // after this are multibyte tokens
	TK_LIMIT,
} token_kind;

typedef struct {
	token_kind kind;
	stringview lexeme;
} token;

vector(token) lexer_tokenize(stringview source);

char* token_name(token t);

#endif//SITEGEN_LEXER_H
