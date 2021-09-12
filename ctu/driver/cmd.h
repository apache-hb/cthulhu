#pragma once

#include "driver.h"

typedef struct {
    const driver_t *driver; 
    vector_t *sources;
    reports_t *reports;
    bool verbose;
} settings_t;

settings_t parse_args(reports_t *reports, int argc, char **argv);
