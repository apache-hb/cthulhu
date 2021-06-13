#pragma once

#include "front.h"

typedef struct {
    const char *path;
    nodes_t *ast;
} scanner_t;

typedef struct YYLTYPE {
  int64_t first_line;
  int64_t first_column;
  int64_t last_line;
  int64_t last_column;
} YYLTYPE;

#define YYLTYPE_IS_DECLARED 1
#define YY_USER_INIT yylloc->first_line = yylloc->first_column = 1;