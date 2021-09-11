#pragma once

#include "driver.h"

typedef struct {
    size_t threads;
    const driver_t *driver; 
    vector_t *sources;
    reports_t *reports;
} settings_t;

settings_t parse_args(reports_t *reports, int argc, char **argv);
