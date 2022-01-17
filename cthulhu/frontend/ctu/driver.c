#include "driver.h"

#include "scan.h"
#include "sema/sema.h"

#include "cthulhu/driver/driver.h"
#include "sema/attrib.h"

static void ctu_init(void) {
    init_attribs();
}

static frontend_t DRIVER = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .init = (init_t)ctu_init,
    .open = (open_t)ctu_open,
    .parse = (parse_t)ctu_compile,
    .analyze = (analyze_t)ctu_analyze
};

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv);
}
