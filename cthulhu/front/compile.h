#pragma once

#include <stdio.h>

#include "front.h"

nodes_t *compile_file(const char *path, FILE *stream);
nodes_t *compile_string(const char *path, const char *text);
