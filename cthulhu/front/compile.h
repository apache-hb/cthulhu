#pragma once

#include <stdio.h>

#include "ast.h"

void max_errors(size_t num);
void add_error(const char *msg, scanner_t *scanner, YYLTYPE loc);
void write_errors(void);
nodes_t *compile_file(const char *path, FILE *stream);
nodes_t *compile_string(const char *path, const char *text);
