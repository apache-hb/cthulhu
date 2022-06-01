#pragma once

#include <gmp.h>

#include <stdbool.h>

#include "base/version-def.h"
#include "std/vector.h"
#include "std/map.h"
#include "report/report.h"

typedef enum
{
    PARAM_BOOL, // either a positional argument or a boolean flag
    PARAM_STRING, // a string of some sort
    PARAM_INT, // an integer
} param_kind_t;

typedef struct arg_t
{
    const char **names;
    size_t totalNames;
    const char *desc;
    param_kind_t kind;
} param_t;

typedef struct
{
    param_kind_t kind;
    bool setByUser;
    union {
        mpz_t digit;
        const char *string;
        bool boolean;
    };
} arg_t;

typedef struct
{
    const char **argv;
    int argc;
    
    const char *description;
    version_t version;

    reports_t *reports;

    vector_t *args;
} arg_parse_config_t;

typedef struct
{
    int exitCode; ///< if non-zero, the program should exit with this code
    map_t *params; ///< provided parameters
    vector_t *extra; ///< provided files 
} arg_parse_result_t;

long get_digit_arg(const arg_t *arg, long other);
const char *get_string_arg(const arg_t *arg, const char *other);
bool get_bool_arg(const arg_t *arg, bool other);

arg_parse_result_t arg_parse(const arg_parse_config_t *config);
