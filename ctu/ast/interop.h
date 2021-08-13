#pragma once

#include "scan.h"

void flex_action(where_t *where, const char *text);
int flex_input(scan_t *scan, char *out, int size);
void flex_init(where_t *where);

#define YY_USER_ACTION \
    flex_action(yylloc, yytext);

#define YY_INPUT(buffer, result, size)          \
    result = flex_input(yyextra, buffer, size); \
    if (result <= 0) { result = YY_NULL; }

#define YY_USER_INIT \
    flex_init(yylloc);

#define YYLTYPE where_t
