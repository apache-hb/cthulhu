#include "cthulhu/driver/driver.h"
#include "cthulhu/ast/compile.h"

#include "cc-bison.h"
#include "cc-flex.h"

/**
 * this actually does both the parsing and semantics
 * C is context sensitive so this is required.
 * as such it returns an hlir tree rather than an ast
 */

static int init(scan_t *extra, void *scanner) {
    return cclex_init_extra(extra, scanner);
}

static void set_in(FILE *fd, void *scanner) {
    ccset_in(fd, scanner);
}

static int parse(scan_t *extra, void *scanner) {
    return ccparse(scanner, extra);
}

static void *scan(const char *text, void *scanner) {
    return cc_scan_string(text, scanner);
}

static void destroy(void *scanner) {
    cclex_destroy(scanner);
}

static callbacks_t CALLBACKS = {
    .init = init,
    .set_in = set_in,
    .parse = parse,
    .scan = scan,
    .destroy = destroy
};

void *cc_parse(reports_t *reports, scan_t *scan) {
    context_t *context = new_context(reports);
    scan_export(scan, context);

    return compile_file(scan, &CALLBACKS);
}

/**
 * just forward the hlir
 */
static hlir_t *cc_sema(reports_t *reports, void *ast) {
    UNUSED(reports);
    return ast;
}

static driver_t DRIVER = {
    .name = "C11",
    .version = "1.0.0",
    .parse = cc_parse,
    .sema = cc_sema
};

int main(int argc, const char **argv) {
    common_init();

    init_types();

    return common_main(argc, argv, DRIVER);
}
