#ifndef MARKLESS_PARSER_H
#define MARKLESS_PARSER_H

#include "sitegen/site_generator.h"
#include "sitegen/markless/document.h"

markless_doc* parse_markless_document(sitegen_context* context, sourcebuffer source);

#endif//MARKLESS_PARSER_H
