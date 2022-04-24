#pragma once

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/io.h"
#include "cthulhu/util/report.h"

#define CT_CALLBACKS(id, prefix)                                                                                       \
    static int prefix##_##id##_##init(scan_t *extra, void *scanner) { return prefix##lex_init_extra(extra, scanner); } \
    static void prefix##_##id##_set_in(FILE *fd, void *scanner) { prefix##set_in(fd, scanner); }                       \
    static int prefix##_##id##_parse(scan_t *extra, void *scanner) { return prefix##parse(scanner, extra); }           \
    static void *prefix##_##id##_scan(const char *text, void *scanner) { return prefix##_scan_string(text, scanner); } \
    static void prefix##_##id##_destroy(void *scanner) { prefix##lex_destroy(scanner); }                               \
    static callbacks_t id = {                                                                                          \
        .init = prefix##_##id##_##init,                                                                                \
        .setIn = prefix##_##id##_set_in,                                                                               \
        .parse = prefix##_##id##_parse,                                                                                \
        .scan = prefix##_##id##_scan,                                                                                  \
        .destroy = prefix##_##id##_destroy,                                                                            \
    }

typedef void *(*parse_t)(reports_t *, scan_t *);
typedef hlir_t *(*analyze_t)(reports_t *, void *);

typedef struct {
    const char *name;
    const char *version;
    parse_t parse;
    analyze_t sema;
} driver_t;

/**
 * @brief initialize the common runtime, always the first function a driver
 * should call
 */
void common_init(void);

/**
 * @brief process the command line and run the compiler
 *
 * @param argc argc from main
 * @param argv argv from main
 * @param driver information about the driver being run
 * @return an exit code to return from main
 */
int common_main(int argc, const char **argv, driver_t driver);

hlir_t *find_module(sema_t *sema, const char *path);
