#pragma once

#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

typedef struct arena_t arena_t;
typedef struct logger_t logger_t;
typedef struct map_t map_t;
typedef struct vector_t vector_t;
typedef struct typevec_t typevec_t;
typedef struct io_t io_t;
typedef struct scan_t scan_t;

/// @brief a preprocessor define
typedef struct cpp_define_t
{
    /// @brief the plain text of the define
    const char *name;

    const char *value;
} cpp_define_t;

typedef struct cpp_instance_t
{
    /// @brief the arena to allocate from
    arena_t *arena;

    /// @brief the logger to use
    logger_t *logger;

    /// @brief the include directories
    vector_t *include_directories;

    /// @brief all encountered defines
    map_t *defines;

    /// @brief all encountered files
    map_t *files;
} cpp_instance_t;

/// @brief creates a new preprocessor define
///
/// @param context the context
/// @param text the text of the define
///
/// @return cpp_define_t the new define
cpp_define_t *cpp_define_new(arena_t *arena, const char *name, const char *text);

/// @brief creates a new preprocessor instance
///
/// @param arena the arena to allocate from
/// @param logger the logger to use
///
/// @return cpp_instance_t the new instance
cpp_instance_t cpp_instance_new(arena_t *arena, logger_t *logger);

void cpp_add_include_dir(cpp_instance_t *instance, const char *path);

/// @brief process a file and all its includes
///
/// @param instance the preprocessor instance
/// @param source the source file
///
/// @return scan_t the preprocessed file
io_t *cpp_process_file(cpp_instance_t *instance, scan_t *source);

END_API
