#pragma once

#include "ctu/util/io.h"
#include "ctu/util/report.h"
#include "ctu/lir/lir.h"

typedef void*(*parse_t)(reports_t*, file_t*);
typedef lir_t*(*analyze_t)(reports_t*, void*);

typedef struct {
    const char *version;
    const char *name;
    parse_t parse;
    analyze_t analyze;
} driver_t;

extern const driver_t CTU;
extern const driver_t PL0;

/**
 * select a driver based on a language name
 * 
 * @param reports report sink
 * @param name the language name to parse
 * 
 * @return the driver or NULL if not found
 */
const driver_t *select_driver(reports_t *reports, const char *name);

/**
 * select a driver for a file based on the file extension
 * 
 * @param reports report sink
 * @param driver the default driver from parse_args
 * @param path the file path
 * 
 * @return the driver or NULL if not found
 */
const driver_t *select_driver_by_extension(reports_t *reports, const driver_t *driver, const char *path);
