#pragma once

#include "ast.h"

#include <stdio.h>

nodes_t *compile_file(const char *path, FILE *stream);
nodes_t *compile_string(const char *path, const char *text);
