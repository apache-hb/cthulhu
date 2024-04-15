// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

#include <stdbool.h>

typedef struct meta_ast_t meta_ast_t;
typedef struct io_t io_t;

#define META_ARGS(...) __VA_ARGS__, NULL

#define META_INIT(argc, argv, name)       \
    do                                    \
    {                                     \
        if (!meta_init(argc, argv, name)) \
        {                                 \
            return meta_get_exit_code();  \
        }                                 \
    } while (0)

CT_BEGIN_API

bool meta_init(int argc, const char **argv, const char *name);
int meta_get_exit_code(void);

void meta_cmdline_arg(const char *name, const char *id, const char *brief, ...);

CT_END_API
