#include "scan.h"
#include "sema.h"

#include "ctu/driver/driver.h"
#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ast.h"
#include "ctu/lir/lir.h"

scan_t pl0_open(reports_t *reports, file_t *file) {
    return scan_file(reports, "PL/0", file);
}

static vector_t *pl0_analyze(reports_t *reports, void *data) {
    return vector_init(pl0_sema(reports, data));
}

static const frontend_t DRIVER = {
    .version = "1.0.0",
    .name = "PL/0",
    .open = (open_t)pl0_open,
    .parse = (parse_t)pl0_compile,
    .analyze = (analyze_t)pl0_analyze
};

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv);
}
