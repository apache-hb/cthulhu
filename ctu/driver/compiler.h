#pragma once

#include "ctu/util/report.h"

typedef struct {
    reports_t *reports;
    vector_t *arenas;
} compiler_t;

compiler_t *new_compiler(size_t arenas);
