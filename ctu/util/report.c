#include "report.h"

#include "str.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

void assert(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

typedef struct {
    level_t level;
    char *message;

    scanner_t *source;
    where_t where;
    
    node_t *node;
} report_t;

static report_t *reports = NULL;
static size_t max_reports = 0;
static size_t num_reports = 0;

static bool already_reported(size_t index) {
    node_t *node = reports[index].node;
    if (!node) {
        return false;
    }

    for (size_t i = 0; i < index; i++) {
        if (reports[i].node == node) {
            return true;
        }
    }

    return false;
}

static void push_report(level_t level, scanner_t *source, where_t where, node_t *node, char *message) {
    report_t it = { level, message, source, where, node };

    if (num_reports < max_reports) {
        reports[num_reports++] = it;
    }
}

static void print_message(level_t level, const char *message) {
    switch (level) {
    case LEVEL_ERROR: fprintf(stderr, COLOUR_RED "error: " COLOUR_RESET); break;
    case LEVEL_WARNING: fprintf(stderr, COLOUR_YELLOW "warning: " COLOUR_RESET); break;
    default: fprintf(stderr, COLOUR_BLUE "other: " COLOUR_RESET); break;
    }
    fprintf(stderr, "%s\n", message);
}

static void print_location(scanner_t *source, where_t where) {
    fprintf(stderr, " => [%s:%" PRId64 ":%" PRId64 "]\n", 
        source->path, 
        where.first_line, where.first_column
    );
}

static void print_padding(size_t size, char sep, bool feed) {
    for (size_t i = 0; i < size; i++) {
        fprintf(stderr, " ");
    }

    if (sep) {
        fprintf(stderr, "%c ", sep);
    }

    if (feed) {
        fprintf(stderr, "\n");
    }
}

static void print_line(scanner_t *source, loc_t line) {
    size_t index = 0;
    char c = source->text[index];
    while (line > 0) {
        c = source->text[index++];
        if (!c) {
            break;
        }
            
        if (c == '\n') {
            line -= 1;
        }
    }

    do {
        c = source->text[index++];
        fprintf(stderr, "%c", c);
    } while (c && c != '\n');
    fprintf(stderr, "\n");
}

static void print_underline(loc_t column, loc_t length) {
    fprintf(stderr, COLOUR_PURPLE);

    for (loc_t i = 0; i < column; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^");
    for (loc_t i = 0; i < length - 1; i++) {
        fprintf(stderr, "~");
    }

    fprintf(stderr, COLOUR_RESET "\n");
}

static void underline_source(scanner_t *source, where_t where) {
    loc_t line = where.first_line;
    loc_t column = where.first_column;
    loc_t length = where.last_column - column;

    char *linestr = format("%" PRId64, line + 1);
    size_t linelen = strlen(linestr);
    size_t padding = linelen + 2;

    print_padding(padding, '|', true);
    fprintf(stderr, " %s | ", linestr);
    print_line(source, line);
    print_padding(padding, '|', false);
    
    /** 
     * this (line ? 1 : 0) works around a strange bug
     * where the column location of the token is off
     * by one on every line *aside* from the first line
     * where its correct.
     */
    print_underline(column - (line ? 1 : 0), length);
}

static void underline_block(scanner_t *source, where_t where) {
    (void)source;
    (void)where;
}

static void outline_source(scanner_t *source, where_t where) {
    if (where.first_line == where.last_line) {
        underline_source(source, where);
    } else {
        underline_block(source, where);
    }
}

static bool print_report(report_t report) {
    print_message(report.level, report.message);
    print_location(report.source, report.where);
    outline_source(report.source, report.where);

    return report.level == LEVEL_ERROR;
}

void report_begin(size_t limit) {
    max_reports = limit;
    reports = malloc(sizeof(report_t) * limit);
}

bool report_end(const char *name) {
    size_t fatal = 0;

    for (size_t i = 0; i < num_reports; i++) {
        if (already_reported(i))
            continue;

        if (print_report(reports[i]))
            fatal += 1;
    }

    if (fatal) {
        fprintf(stderr, "aborting after %s state due to %zu error(s)\n", name, fatal);
    }

    return fatal != 0;
}

void reportf(level_t level, node_t *node, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    push_report(level, node->scanner, node->where, node, formatv(fmt, args));
    va_end(args);
}

void report(level_t level, scanner_t *source, where_t where, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    push_report(level, source, where, NULL, formatv(fmt, args));
    va_end(args);
}
