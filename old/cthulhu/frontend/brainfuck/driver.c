#include "cthulhu/driver/cmd.h"
#include "cthulhu/driver/driver.h"
#include "cthulhu/ast/compile.h"

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

enum {
    BF_TAPE_SIZE = 0,
    BF_MAX
};

static vector_t *bf_build(reports_t *reports, settings_t *settings, void *data) {
    UNUSED(reports);
    UNUSED(settings);
    UNUSED(data);

    size_t tape = 0x2000;
    parsed_arg_t *arg = get_arg(reports, settings, BF_TAPE_SIZE, ARG_UINT);
    if (arg != NULL) {
        tape = mpz_get_ui(arg->digit);
    }
    printf("tape-size: %zu\n", tape);

    return vector_new(0);
}

void bferror(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}

int main(int argc, char **argv) {
    vector_t *args = vector_of(BF_MAX);
    arg_t *ts = new_arg(ARG_UINT, "--tape-size", "-TS", "Configure the number of cells in the tape");
    vector_set(args, BF_TAPE_SIZE, ts);

    const frontend_t driver = {
        .version = "1.0.0",
        .name = "brainfuck",
        .open = (open_t)bf_open,
        .parse = (parse_t)bf_compile,
        .analyze = (analyze_t)bf_build,
        .args = args
    };

    return common_main(&driver, argc, argv);
}
