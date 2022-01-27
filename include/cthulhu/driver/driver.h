#pragma once

#include "cthulhu/util/report.h"
#include "cthulhu/util/io.h"
#include "cthulhu/hlir/hlir.h"

typedef void*(*parse_t)(scan_t*);
typedef hlir_t*(*analyze_t)(reports_t*, void*);

typedef struct {
    const char *name;
    const char *version;
    parse_t parse;
    analyze_t sema;
} driver_t;

void common_init(void);

int common_main(int argc, const char **argv, driver_t driver);
