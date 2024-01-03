#pragma once

#include "core/text.h"
#include "scan/node.h"

typedef struct cpp_ast_t cpp_ast_t;

typedef enum cpp_kind_t
{
    eCppIdent,
    eCppCount
} cpp_kind_t;

typedef struct cpp_ast_t
{
    cpp_kind_t kind;
    const node_t *node;

    union {
        text_t ident;
    };
} cpp_ast_t;

cpp_ast_t *cpp_ast_ident(const scan_t *scan, where_t where, text_t text);
