#pragma once

#include "driver.h"

typedef union {
    arg_type_t type;
    union {
        bool boolean;
        mpz_t digit;
    };
} parsed_arg_t;

typedef struct settings_t {
    const backend_t *backend; /// codegen backend
    vector_t *sources; /// all source files
    vector_t *headers; /// include paths
    reports_t *reports; /// error report sink
    bool verbose; /// enable verbose logging
    bool ir; /// enable IR output
    const char *output; /// output file name
    vector_t *extra; /// extra arguments
} settings_t;

/**
 * parse command line arguments into settings_t
 * 
 * @param arena allocation arena
 * @param reports report buffer to error into
 * @param frontend the compiler frontend 
 * @param argc number of arguments
 * @param argv array of arguments
 * 
 * @return parsed settings
 */
settings_t parse_args(reports_t *reports, const frontend_t *frontend, int argc, char **argv);

parsed_arg_t *get_arg(reports_t *reports, settings_t *settings, size_t arg, arg_type_t type);
