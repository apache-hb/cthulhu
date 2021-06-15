#include "report.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static uint64_t total = 0;
static FILE *dbg = NULL;

FILE *get_debug(void) {
    return dbg;
}

void set_debug(FILE *file) {
    dbg = file;
}

void debugf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(dbg, fmt, args);
    va_end(args);
}

void add_fail(void) {
    total++;
}

void reportf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    reportv(fmt, args);
    va_end(args);
}

void reportv(const char *fmt, va_list args) {
    add_fail();
    fprintf(stderr, COLOUR_RED "error: " COLOUR_RESET);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

bool check_errors(const char *stage) {
    if (total > 0) {
        reportf("%s: aborting due to %llu error(s)", stage, total);
        return true;
    }
    return false;
}
