#include "driver.h"

#include "ctu/util/str.h"

#include <string.h>

#include "ctu/frontend/pl0/driver.h"
#include "ctu/frontend/ctu/driver.h"
#include "ctu/frontend/c/driver.h"

const driver_t PL0 = {
    .version = "0.0.1",
    .name = "PL/0",
    .parse = (parse_t)pl0_parse,
    .analyze = (analyze_t)pl0_analyze
};

const driver_t CTU = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .parse = (parse_t)ctu_parse,
    .analyze = (analyze_t)ctu_analyze
};

/*
const driver_t C = {
    .version = "0.0.1",
    .name = "C",
    .parse = (parse_t)c_parse,
    .analyze = (analyze_t)c_analyze
};
*/
const driver_t *select_driver(reports_t *reports, const char *name) {
    if (name == NULL) {
        report2(reports, ERROR, NULL, "no driver specified");
        return NULL;
    }

    if (strcmp(name, "pl0") == 0) {
        return &PL0;
    } else if (strcmp(name, "ctu") == 0) {
        return &CTU;
    } else {
        report2(reports, ERROR, NULL, "unknown driver: %s", name);
        return NULL;
    }
}

const driver_t *select_driver_by_extension(reports_t *reports, const driver_t *driver, const char *path) {
    if (driver != NULL) {
        return driver;
    }

    if (endswith(path, ".pl0")) {
        return &PL0;
    } else if (endswith(path, ".ct")) {
        return &CTU;
    } else {
        report2(reports, ERROR, NULL, "unknown extension on input: %s", path);
        return NULL;
    }
}
