#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "cthulhu/mediator/interface.h"

typedef struct logger_t reports_t;
typedef struct map_t map_t;
typedef struct ap_t ap_t;
typedef struct vector_t vector_t;

typedef struct runtime_t
{
    int argc;
    const char **argv;

    mediator_t *mediator;
    lifetime_t *lifetime;

    reports_t *reports;
    ap_t *ap;

    bool emitSSA;

    bool warnAsError;
    size_t reportLimit;

    const char *sourceOut;
    const char *headerOut;

    vector_t *sourcePaths;
} runtime_t;

runtime_t cmd_parse(reports_t *reports, mediator_t *mediator, lifetime_t *lifetime, int argc, const char **argv);
