#pragma once

#include "cthulhu/util/report.h"
#include "cthulhu/util/io.h"
#include "cthulhu/hlir/hlir.h"

#define CT_CALLBACKS(id, prefix) \
    static int prefix##_##id##_##init(scan_t *extra, void *scanner) { return prefix##lex_init_extra(extra, scanner); } \
    static void prefix##_##id##_set_in(FILE *fd, void *scanner) { prefix##set_in(fd, scanner); } \
    static int prefix##_##id##_parse(scan_t *extra, void *scanner) { return prefix##parse(scanner, extra); } \
    static void *prefix##_##id##_scan(const char *text, void *scanner) { return prefix##_scan_string(text, scanner); } \
    static void prefix##_##id##_destroy(void *scanner) { prefix##lex_destroy(scanner); } \
    static callbacks_t id = { \
        .init = prefix##_##id##_##init, \
        .set_in = prefix##_##id##_set_in, \
        .parse = prefix##_##id##_parse, \
        .scan = prefix##_##id##_scan, \
        .destroy = prefix##_##id##_destroy \
    }

typedef void*(*parse_t)(reports_t*, scan_t*);
typedef hlir_t*(*analyze_t)(reports_t*, void*);

typedef struct {
    const char *name;
    const char *version;
    parse_t parse;
    analyze_t sema;

    const char *stdlib_path;
} driver_t;

void common_init(void);

int common_main(int argc, const char **argv, driver_t driver);

hlir_t *find_module(vector_t *path);
