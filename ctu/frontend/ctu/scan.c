#include "scan.h"

#include "ctu/util/report.h"

#include "ctu-bison.h"
#include "ctu-flex.h"

static int init(scan_t *extra, void *scanner) {
    return ctulex_init_extra(extra, scanner);
}

static void set_in(FILE *fd, void *scanner) {
    ctuset_in(fd, scanner);
}

static int parse(void *scanner, void *extra) {
    return ctuparse(scanner, extra);
}

static void *scan(const char *text, void *scanner) {
    return ctu_scan_string(text, scanner);
}

static void destroy(void *scanner) {
    ctulex_destroy(scanner);
}

static callbacks_t CALLBACKS = {
    .init = init,
    .set_in = set_in,
    .parse = parse,
    .scan = scan,
    .destroy = destroy
};

static const char *LANGUAGE = "cthulhu";

ctu_t *ctu_compile(reports_t *reports, file_t *fd) {
    scan_t *scan = scan_file(reports, LANGUAGE, fd);
    ctu_t *node = compile_file(scan, &CALLBACKS);
    //scan_delete(scan);
    return node;
}

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}
