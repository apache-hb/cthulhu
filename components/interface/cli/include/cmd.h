#pragma once

#include <stdbool.h>

#include "cthulhu/mediator/mediator.h"

typedef struct reports_t reports_t;
typedef struct map_t map_t;
typedef struct ap_t ap_t;
typedef struct vector_t vector_t;

typedef struct runtime_t 
{
    int argc;
    const char **argv;

    reports_t *reports;
    ap_t *ap;
    mediator_t *mediator;

    vector_t *languages;
    vector_t *plugins;

    bool emitSSA;

    bool warnAsError;
    size_t reportLimit;

    const char *sourceOut;
    const char *headerOut;

    vector_t *sourcePaths;

    vector_t *unknownArgs;
} runtime_t;

runtime_t cmd_parse(mediator_t *mediator, int argc, const char **argv);
