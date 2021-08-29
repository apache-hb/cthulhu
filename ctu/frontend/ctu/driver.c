#include "driver.h"

#include "scan.h"

node_t *ctu_driver(file_t *file) {
    return ctu_compile(file);
}
