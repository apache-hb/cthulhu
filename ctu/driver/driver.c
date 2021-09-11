#include "driver.h"

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

const driver_t C = {
    .version = "0.0.1",
    .name = "C",
    .parse = (parse_t)c_parse,
    .analyze = (analyze_t)c_analyze
};

const driver_t INVALID = {
    .version = "1.0.0",
    .name = "Invalid",
    .parse = NULL,
    .analyze = NULL
};
