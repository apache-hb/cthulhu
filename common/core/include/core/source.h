#pragma once

#include "core/compiler.h"

typedef struct source_t
{
    size_t line;
    const char *function;
    const char *file;
} source_t;

#define CTU_HERE() (source_t){ .line = __LINE__, .function = FUNCNAME, .file = __FILE__ }
