#pragma once

#include "ast.h"

#include <stdio.h>

/**
 * scanner is an optional out param
 * that can be used to get the internal scanner
 */

list_t *compile_file(const char *path, FILE *stream, scanner_t **scanner);
list_t *compile_string(const char *path, const char *text, scanner_t **scanner);
