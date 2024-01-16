#pragma once

#include "core/compiler.h"

typedef struct arena_t arena_t;
typedef struct logger_t logger_t;
typedef struct fs_t fs_t;

typedef struct vector_t vector_t;
typedef struct map_t map_t;

typedef struct cfg_group_t cfg_group_t;

BEGIN_API

/// @defgroup target Target output helper library
/// @brief Target output helper library
/// @ingroup runtime
/// @{

/// @brief output folder structure
typedef enum file_layout_t
{
    /// @brief create subfolders matching the module path
    eFileLayoutTree,

    /// @brief name files with the full module path
    eFileLayoutFlat,

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

    cfg_group_t *config;
} emit_t;

/// @brief a target builder
typedef void (*target_build_t)(emit_t *emit, vector_t *modules, map_t *deps);

// TODO: currently the config is mutated so we need to make a new one
//       every invocation. thats not ideal, we should return something immutable
//       and pass in a copy with modifications.

/// @brief get the config for this target
typedef cfg_group_t *(*target_config_t)(cfg_group_t *root);

// return a map_t<const char*, attrib_t>
typedef map_t *(*target_attribs_t)(arena_t *arena);

// TODO: work on attributes
typedef struct attrib_t
{
    const char *name;
} attrib_t;

/// @brief a codegen target
typedef struct target_t
{
    /// @brief the name of the target
    const char *name;

    /// @brief the target builder function
    target_build_t fn_build;

    target_config_t fn_get_config;

    target_attribs_t fn_get_attribs;
} target_t;

#if CTU_BUILD_SHARED
#   define CTU_TARGET_ENTRY(mod) CT_TARGET_API const target_t *target_main(void) { return &mod; }
#else
#   define CTU_TARGET_ENTRY(mod)
#endif

/// @}

END_API
