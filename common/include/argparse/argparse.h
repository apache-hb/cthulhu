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

    PARAM_TOTAL
} param_kind_t;

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
    const char **names;
    size_t totalNames;
    const char *desc;
    param_kind_t kind;

    const arg_t *generatedArg; ///< the argument that this parameter generates
} param_t;

typedef struct 
{
    const char *name;
    const char *desc;
    vector_t *params;
} group_t;

typedef struct
{
    const char **argv;
    int argc;
    
    const char *description;
    version_t version;

    reports_t *reports;

    vector_t *groups; ///< vec<group_t>
} arg_parse_config_t;

typedef struct
{
    int exitCode; ///< if not INT_MAX, the program should exit with this code

    report_config_t reportConfig; ///< user configured report config

    map_t *params; ///< provided parameters
    vector_t *extra; ///< provided files 
} arg_parse_result_t;

long get_digit_arg(const param_t *arg, long other);
const char *get_string_arg(const param_t *arg, const char *other);
bool get_bool_arg(const param_t *arg, bool other);

group_t *new_group(const char *name, const char *desc, vector_t *params);
param_t *new_param(param_kind_t kind, const char *desc, const char **names, size_t total);
arg_parse_result_t arg_parse(const arg_parse_config_t *config);

#define ADD_FLAG(params, kind, desc, ...) do { \
    static const char *names[] = __VA_ARGS__; \
    param_t *param = new_param(kind, desc, names, sizeof(names) / sizeof(const char *)); \
    vector_push(&params, param); \
    } while (0)
