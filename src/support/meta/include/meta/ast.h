// SPDX-License-Identifier: LGPL-3.0-only
#pragma once

#include "core/text.h"
#include "core/where.h"
#include "std/typed/vector.h"

#include <stdbool.h>

typedef struct node_t node_t;

typedef struct meta_ast_t meta_ast_t;

typedef enum meta_kind_t {
    eMetaFile,

    eMetaCmdFlag, // --flag
    eMetaCmdCombo, // --flag=<value1|value2|value3>

    eMetaCount,
} meta_kind_t;

typedef struct meta_ast_t {
    meta_kind_t kind;

    const node_t *node;

    union {
        // eMetaFile
        struct {
            text_view_t prefix;

            typevec_t all_flags; // typevec_t<meta_ast_t>
        };

        // any flag
        struct {
            typevec_t flags; // typevec_t<text_view_t>

            // eMetaCmdFlag
            bool default_value;

            // eMetaCmdCombo
            typevec_t combo_choices; // typevec_t<meta_ast_t>
            size_t default_choice; // SIZE_MAX if no default
        };
    };
} meta_ast_t;

meta_ast_t *meta_ast_file(
    const node_t *node, where_t where,
    text_view_t prefix,
    typevec_t all_flags);

meta_ast_t *meta_ast_flag(
    const node_t *node, where_t where,
    typevec_t flags,
    bool default_value);

meta_ast_t *meta_ast_combo(
    const node_t *node, where_t where,
    typevec_t flags,
    typevec_t combo_choices,
    text_view_t default_choice);
