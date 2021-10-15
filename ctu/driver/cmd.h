#pragma once

#include "driver.h"

typedef struct {
    const backend_t *backend;
    vector_t *sources;
    reports_t *reports;
    bool verbose;
    bool ir;
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
settings_t parse_args(arena_t *arena, reports_t *reports, const frontend_t *frontend, int argc, char **argv);
