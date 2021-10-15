#include "driver.h"

#include "scan.h"
#include "sema/sema.h"

#include "ctu/driver/driver.h"
#include "sema/attrib.h"

static scan_t ctu_open(reports_t *reports, file_t *file) {
    return scan_file(reports, "cthulhu", file);
}

static const frontend_t DRIVER = {
    .version = "0.0.1",
    .name = "Cthulhu",
    .open = (open_t)ctu_open,
    .parse = (parse_t)ctu_compile,
    .analyze = (analyze_t)ctu_sema
};

static void ctu_init(void) {
    init_attribs();
}

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv, ctu_init);
}
