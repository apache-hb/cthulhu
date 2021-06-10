#include "report.h"

#include <stdio.h>
#include <stdarg.h>

#define RED "\e[1;31m"
#define RESET "\e[0m"

static uint64_t total = 0;

static void error(void) {
    total++;
    fprintf(stderr, RED "error: " RESET);
}

uint64_t errors(void) {
    return total;
}

void report(const char *fmt) {
    error();

    fprintf(stderr, "%s\n", fmt);
}

void reportf(const char *fmt, ...) {
    error();

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
