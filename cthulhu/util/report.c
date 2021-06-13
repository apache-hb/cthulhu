#include "report.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define RED "\x1B[1;31m"
#define RESET "\x1B[0m"

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

void reportf(const char *fmt, ...) {
    total++;
    fprintf(stderr, RED "error: " RESET);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void check_errors(const char *stage) {
    if (total > 0) {
        reportf("%s: aborting due to %llu previous error(s)", stage, total);
        exit(1);
    }
}
