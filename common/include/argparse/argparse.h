#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "base/analyze.h"
#include "base/version-def.h"

#include "report/report.h"

typedef struct map_t map_t;

typedef enum
{
    eParamBool,   // either a positional argument or a boolean flag
    eParamString, // a string of some sort
    eParamInt,    // an integer

    eParamTotal
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
    status_t exitCode;

    bool verboseEnabled;
    size_t reportLimit;
    bool warningsAsErrors;

    vector_t *files;

    map_t *params;

    /// private data
    reports_t *reports;
    const char *currentName;
    arg_t *currentArg;
    map_t *lookup;
} argparse_t;

NODISCARD
param_t *int_param(const char *desc, const char **names, size_t total);

NODISCARD
param_t *string_param(const char *desc, const char **names, size_t total);

NODISCARD
param_t *bool_param(const char *desc, const char **names, size_t total);

NODISCARD
group_t *new_group(const char *name, const char *desc, vector_t *params);

NODISCARD CONSTFN long get_digit_arg(const argparse_t *argparse, const param_t *arg, long other);

NODISCARD CONSTFN const char *get_string_arg(const argparse_t *argparse, const param_t *arg, const char *other);

NODISCARD CONSTFN bool get_bool_arg(const argparse_t *argparse, const param_t *arg, bool other);

void argparse_init(void);

NODISCARD CONSTFN argparse_t parse_args(const argparse_config_t *config);

NODISCARD CONSTFN bool should_exit(const argparse_t *argparse);
