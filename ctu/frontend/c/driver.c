#include "ctu/driver/driver.h"
#include "ctu/util/util.h"
#include "ctu/util/report.h"
#include "ctu/ast/compile.h"
#include "ast.h"
#include "ctu/lir/lir.h"

static scan_t c_open(reports_t *reports, file_t *file) {
    return scan_file(reports, "C11", file);
}

c_t *c_parse(scan_t *scan) {
    UNUSED(scan);
    CTASSERT(false, "c-parse not implemented");
    return NULL;
}

lir_t *c_analyze(reports_t *reports, c_t *node) {
    UNUSED(reports);
    UNUSED(node);
    CTASSERT(false, "c-analyze not implemented");
    return NULL;
}

static const frontend_t DRIVER = {
    .version = "0.0.1",
    .name = "C",
    .open = (open_t)c_open,
    .parse = (parse_t)c_parse,
    .analyze = (analyze_t)c_analyze
};

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv);
}
