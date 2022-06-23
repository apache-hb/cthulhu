#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "base/version-def.h"

typedef struct reports_t reports_t;
typedef struct vector_t vector_t;
typedef struct map_t map_t;

typedef enum
{
    PARAM_BOOL, // either a positional argument or a boolean flag
    PARAM_STRING, // a string of some sort
    PARAM_INT, // an integer

    PARAM_TOTAL
} param_kind_t;

/**
 * a user provided argument from the command line
 */
typedef struct 
{
    param_kind_t kind;
    bool setByUser;
    union {
        long digit;
        const char *string;
        bool boolean;
    };
} arg_t;

/**
 * an option that can be specified on the command line 
 */
typedef struct
{
    const char **names;
    size_t totalNames;
    const char *description;
    param_kind_t kind;
} param_t;

typedef struct
{
    const char *name;
    const char *description;

    vector_t *params;
} group_t;

typedef struct
{
    int argc;
    const char **argv;

    const char *description;
    version_t version;

    reports_t *reports;

    vector_t *groups;
} argparse_config_t;

typedef struct argparse_t
{
    int exitCode;

    bool verboseEnabled;
    size_t reportLimit;
    bool warningsAsErrors;

    map_t *params;

    vector_t *files;

    /// private data
    reports_t *reports;
    const char *currentName;
    arg_t *currentArg;
} argparse_t;

param_t *int_param(const char *desc, const char **names, size_t total);
param_t *string_param(const char *desc, const char **names, size_t total);
param_t *bool_param(const char *desc, const char **names, size_t total);

group_t *new_group(const char *name, const char *desc, vector_t *params);

long get_digit(const argparse_t *argparse, const param_t *arg, long other);
const char *get_string(const argparse_t *argparse, const param_t *arg, const char *other);
bool get_bool(const argparse_t *argparse, const param_t *arg, bool other);

argparse_t parse_args(const argparse_config_t *config);

bool should_exit(const argparse_t *argparse);
