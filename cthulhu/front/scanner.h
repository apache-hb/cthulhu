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

typedef struct YYLTYPE {
  size_t distance;
  int64_t first_line;
  int64_t first_column;
  int64_t last_line;
  int64_t last_column;
} YYLTYPE;

#define YYLTYPE_IS_DECLARED 1
#define YY_USER_INIT flex_init(yylloc);
void flex_init(YYLTYPE *loc);
void scan_reportf(YYLTYPE *where, scanner_t *x, const char *fmt, ...);
int flex_provide(YYLTYPE *loc, scanner_t *x, char *buf);
