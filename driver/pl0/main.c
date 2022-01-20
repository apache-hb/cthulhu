#include "cthulhu/driver/driver.h"
#include "cthulhu/ast/compile.h"

#include "sema.h"

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

void *pl0_parse(scan_t *scan) {
    return compile_file(scan, &CALLBACKS);
}

static driver_t DRIVER = {
    .name = "PL/0",
    .version = "2.0.0",
    .parse = pl0_parse,
    .sema = pl0_sema
};

int main(int argc, const char **argv) {
    pl0_init();
    
    return common_main(argc, argv, DRIVER);
}
