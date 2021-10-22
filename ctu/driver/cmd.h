#pragma once

#include "driver.h"

typedef struct {
    const backend_t *backend; /// codegen backend
    vector_t *sources; /// all source files
    vector_t *headers; /// include paths
    reports_t *reports; /// error report sink
    bool verbose; /// enable verbose logging
    bool ir; /// enable IR output
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
