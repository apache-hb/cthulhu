#include "ctu/driver/driver.h"
#include "ctu/ast/compile.h"

#include "bf-bison.h"
#include "bf-flex.h"

static int init(scan_t *extra, void *scanner) {
    return bflex_init_extra(extra, scanner);
}

static void set_in(FILE *fd, void *scanner) {
    bfset_in(fd, scanner);
}

static int parse(scan_t *extra, void *scanner) {
    return bfparse(scanner, extra);
}

static void *scan(const char *text, void *scanner) {
    return bf_scan_string(text, scanner);
}

static void destroy(void *scanner) {
    bflex_destroy(scanner);
}

static callbacks_t CALLBACKS = {
    .init = init,
    .set_in = set_in,
    .parse = parse,
    .scan = scan,
    .destroy = destroy
};

static scan_t bf_open(reports_t *reports, file_t *file) {
    return scan_file(reports, "brainfuck", file);
}

static vector_t *bf_compile(scan_t *scan) {
    return compile_file(scan, &CALLBACKS);
}

static vector_t *bf_build(reports_t *reports, void *data) {
    UNUSED(reports);
    UNUSED(data);

    return vector_new(0);
}

void bferror(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}

static const frontend_t DRIVER = {
    .version = "1.0.0",
    .name = "brainfuck",
    .open = (open_t)bf_open,
    .parse = (parse_t)bf_compile,
    .analyze = (analyze_t)bf_build
};

int main(int argc, char **argv) {
    return common_main(&DRIVER, argc, argv);
}
