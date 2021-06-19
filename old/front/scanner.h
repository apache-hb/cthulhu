#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
  /* the name of this file */
  const char *path;
  /* output ast nodes */
  void *ast;

  /* file stream data */
  void *data;
  int (*next)(void*);

  /* source code buffer for error reporting */
  char *text;
  size_t len;
  size_t size;
} scanner_t;

typedef int64_t loc_t;

typedef struct YYLTYPE {
  loc_t first_line;
  loc_t first_column;
  loc_t last_line;
  loc_t last_column;
} YYLTYPE;

#define YYLTYPE_IS_DECLARED 1
#define YY_USER_INIT flex_init(yylloc);

void flex_init(YYLTYPE *loc);
int flex_provide(scanner_t *x, char *buf);
