#pragma once

#include "cthulhu/ast/scan.h"
#include "cthulhu/ast/ast.h"

#include "cthulhu/util/vector.h"

typedef enum
{
    AST_LINE_COMMENT,
    AST_BLOCK_COMMENT,

    AST_TOTAL
} ast_kind_t;

typedef struct 
{
    ast_kind_t kind;
    node_t node;

    union {
        vector_t *comments;

        const char *lineComment;

        struct {
            const char *blockCommentStart;
            const char *blockCommentEnd;
        };
    };
} ast_t;

ast_t *gen_grammar(scan_t *scan, where_t where, vector_t *comments);

ast_t *gen_line_comment(scan_t *scan, where_t where, const char *lineComment);
ast_t *gen_block_comment(scan_t *scan, where_t where, const char *start, const char *end);

#define GENLTYPE where_t
