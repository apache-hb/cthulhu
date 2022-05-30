#pragma once

#include <gmp.h>

#include <stdbool.h>

#include "base/version-def.h"
#include "std/vector.h"
#include "std/map.h"

typedef enum
{
    ARG_BOOL, // either a positional argument or a boolean flag
    ARG_STRING, // a string of some sort
    ARG_INT,
} arg_kind_t;

typedef struct arg_t
{
    const char **names;
    size_t totalNames;
    const char *desc;
    arg_kind_t kind;
} arg_t;

typedef struct
{
    const char *argv;
    int argc;
    
    const char *description;
    version_t version;

    vector_t *warnings;

    vector_t *args;
} arg_parse_config_t;

typedef struct
{
    bool success;
    map_t *params; ///< provided parameters
    vector_t *extra; ///< provided files 
} arg_parse_result_t;

arg_parse_result_t arg_parse(const arg_parse_config_t *config);
