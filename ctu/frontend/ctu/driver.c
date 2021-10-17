#include "driver.h"

#include "scan.h"
#include "sema/sema.h"

#include "ctu/driver/driver.h"
#include "sema/attrib.h"

static scan_t ctu_open(reports_t *reports, file_t *file) {
    return scan_file(reports, "cthulhu", file);
}

static void ctu_init(void) {
    init_attribs();
}

static const frontend_t DRIVER = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .init = (init_t)ctu_init,
    .open = (open_t)ctu_open,
    .parse = (parse_t)ctu_compile,
    .analyze = (analyze_t)ctu_sema
};

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv);
}
