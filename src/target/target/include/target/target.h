// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <ctu_target_api.h>

#include "core/compiler.h"
#include "core/analyze.h"

typedef struct arena_t arena_t;
typedef struct logger_t logger_t;
typedef struct fs_t fs_t;

typedef struct vector_t vector_t;
typedef struct map_t map_t;

typedef struct cfg_group_t cfg_group_t;

CT_BEGIN_API

/// @defgroup target Target output helper library
/// @brief Target output helper library
/// @ingroup runtime
/// @{

/// @brief output folder structure
typedef enum file_layout_t
{
#define FILE_LAYOUT(ID, STR) ID,
#include "target.inc"
    eFileLayoutCount
} file_layout_t;

/// @brief target code emitter options
typedef struct emit_t
{
    /// @brief arena to use
    arena_t *arena;

    /// @brief reporting sink
    logger_t *logger;

    /// @brief output filesystem
    fs_t *fs;

    /// @brief output layout
    file_layout_t layout;
} emit_t;

CT_CONSTFN
CT_TARGET_API const char *file_layout_str(IN_DOMAIN(<, eFileLayoutCount) file_layout_t layout);

/// @}

CT_END_API
