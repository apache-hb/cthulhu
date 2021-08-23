#include "driver.h"

#include "ctu/util/report.h"

#include "scan.h"

node_t *pl0_driver(file_t *file) {
    return pl0_compile(file);
}
