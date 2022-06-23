#pragma once

#include "platform/file.h"

typedef struct reports_t reports_t;
typedef struct ast_t ast_t;

typedef struct {
    reports_t *reports;
    const ast_t *root;

    const char *path;

    bool enableVsCode;

    const char *id;
    const char *upperId;
} emit_t;

void emit(emit_t *config);
