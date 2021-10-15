#include "scan.h"

#include "ctu/util/report.h"

#include "pl0-bison.h"
#include "pl0-flex.h"

static int init(scan_t *extra, void *scanner) {
    return pl0lex_init_extra(extra, scanner);
}

static void set_in(FILE *fd, void *scanner) {
    pl0set_in(fd, scanner);
}

static int parse(scan_t *extra, void *scanner) {
    return pl0parse(scanner, extra);
}

static void *scan(const char *text, void *scanner) {
    return pl0_scan_string(text, scanner);
}

static void destroy(void *scanner) {
    pl0lex_destroy(scanner);
}

static callbacks_t CALLBACKS = {
    .init = init,
    .set_in = set_in,
    .parse = parse,
    .scan = scan,
    .destroy = destroy
};

pl0_t *pl0_compile(scan_t *scan) {
    return compile_file(scan, &CALLBACKS);
}

void pl0error(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}
