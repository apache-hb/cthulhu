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

node_t *ctu_compile(file_t *fd) {
    scan_t *scan = scan_file(LANGUAGE, fd);
    node_t *node = compile_file(scan, &CALLBACKS);
    return node;
}

void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg) {
    (void)state;

    reportf(ERROR, scan, *where, "%s", msg);
}