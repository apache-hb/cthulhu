#pragma once

typedef struct arena_t arena_t;
typedef struct logger_t logger_t;
typedef struct io_t io_t;
typedef struct scan_t scan_t;
typedef struct vector_t vector_t;
typedef struct map_t map_t;
typedef struct cpp_t cpp_t;

typedef struct cpp_define_t cpp_define_t;

typedef struct cpp_config_t
{
    arena_t *arena;
    logger_t *logger;

    vector_t *include_directories;
    size_t max_include_depth;

    map_t *defines;
} cpp_config_t;

io_t *cpp_preprocess(cpp_config_t config, scan_t *input);
