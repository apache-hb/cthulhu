#include "report.h"

#include "str.h"
#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct {
    level_t level;
    char *message;

    scanner_t *source;
    where_t where;
    
    /**
     * underline message
     * may be null
     */
    const char *underline;

    /**
     * note
     * may be null
     */
    const char *note;

    node_t *node;

    /**
     * unique tag
     */
    const void *tag;
} report_t;

static bool eager_report = false;
static report_t *reports = NULL;
static size_t max_reports = 0;
static size_t num_reports = 0;
static bool internal = false;

static const void *get_tag(size_t idx) {
    const void *tag = reports[idx].tag;
    if (!tag) {
        tag = reports[idx].node;
    }
    return tag;
}

static bool already_reported(size_t index) {
    const void *tag = get_tag(index);
    if (!tag) {
        return false;
    }

    for (size_t i = 0; i < index; i++) {
        if (get_tag(i) == tag) {
            return true;
        }
    }

    return false;
}

static void print_message(level_t level, const char *message) {
    switch (level) {
    case LEVEL_INTERNAL: fprintf(stderr, COLOUR_CYAN "internal compiler error: " COLOUR_RESET); break;
    case LEVEL_ERROR: fprintf(stderr, COLOUR_RED "error: " COLOUR_RESET); break;
    case LEVEL_WARNING: fprintf(stderr, COLOUR_YELLOW "warning: " COLOUR_RESET); break;
    default: fprintf(stderr, COLOUR_BLUE "other: " COLOUR_RESET); break;
    }
    fprintf(stderr, "%s\n", message);
}

static void print_location(scanner_t *source, where_t where) {
    fprintf(stderr, " => [%s:%" PRId64 ":%" PRId64 "]\n", 
        source->path, 
        where.first_line + 1, /* most text editors columns start at 1 */
        where.first_column
    );
}

static void print_padding(size_t size, bool feed) {
    for (size_t i = 0; i < size + 1; i++) {
        fprintf(stderr, " ");
    }

    fprintf(stderr, "| ");

    if (feed) {
        fprintf(stderr, "\n");
    }
}

static void print_line(scanner_t *source, loc_t line) {
    size_t index = 0;
    char c = '\0';
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
    
    if (c != '\n')
        fprintf(stderr, "\n");
}

static void print_underline(loc_t column, loc_t length, const char *msg) {
    fprintf(stderr, COLOUR_PURPLE);

    for (loc_t i = 0; i < column; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "╰");
    for (loc_t i = 0; i < length - 1; i++) {
        fprintf(stderr, "─");
    }

    if (msg) {
        fprintf(stderr, " %s", msg);
    }

    fprintf(stderr, COLOUR_RESET "\n");
}

static void print_line_indicator(const char *text, size_t len, size_t padding) {
    for (size_t i = 0; i < (padding - len); i++)
        fprintf(stderr, " ");

    fprintf(stderr, "%s | ", text);
}

static void underline_source(scanner_t *source, where_t where, const char *msg) {
    loc_t line = where.first_line;
    loc_t column = where.first_column;
    loc_t length = where.last_column - column;

    char *linestr = format("%" PRId64, line + 1);
    size_t linelen = strlen(linestr);
    size_t padding = linelen + 1;

    print_padding(padding, true);
    print_line_indicator(linestr, linelen, padding);
    print_line(source, line);
    print_padding(padding, false);
    print_underline(column, length, msg);
}

static void outline_source(scanner_t *source, where_t where, const char *msg) {
    if (where.first_line == where.last_line) {
        underline_source(source, where, msg);
    } else {
        /* TODO: better multiline span reporting */
        fprintf(stderr, "spanning line %" PRId64 " to line %" PRId64 "\n", 
            where.first_line, where.last_line
        );
        if (msg) {
            fprintf(stderr, "message: %s\n", msg);
        }
    }
}

/**
 * print an error report to the console
 * return true if the report is fatal
 */
static bool print_report(report_t report) {
    print_message(report.level, report.message);
    
    if (report.source) {
        print_location(report.source, report.where);
        outline_source(report.source, report.where, report.underline);
    }

    if (report.note) {
        fprintf(stderr, COLOUR_GREEN "note:" COLOUR_RESET " %s\n", report.note);
    }

    return report.level == LEVEL_ERROR 
        || report.level == LEVEL_INTERNAL;
}

static reportid_t push_report(level_t level, scanner_t *source, where_t where, node_t *node, char *message) {
    report_t it = { level, message, source, where, NULL, NULL, node, node };

    if (level == LEVEL_INTERNAL) {
        internal = true;
    }

    if (eager_report) {
        print_report(it);
    }
    
    if (num_reports < max_reports) {
        reports[num_reports] = it;
        return num_reports++;
    }

    return INVALID_REPORT;
}

void report_begin(size_t limit, bool eager) {
    max_reports = limit;
    eager_report = eager;
    reports = ctu_realloc(reports, sizeof(report_t) * limit);
}

bool report_end(const char *name) {
    size_t fatal = 0;

    for (size_t i = 0; i < num_reports; i++) {
        /* dont report duplicate errors */
        if (already_reported(i))
            continue;

        /* count the number of fatal errors */
        if (print_report(reports[i]))
            fatal += 1;
    }

    if (fatal) {
        fprintf(stderr, "aborting after %s stage due to %zu error(s)\n", name, fatal);
    }

    return fatal != 0;
}

const where_t NOWHERE = { 0, 0, 0, 0 };

void ensure(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    push_report(LEVEL_ERROR, NULL, NOWHERE, NULL, formatv(fmt, args));
    va_end(args);
}

void assert(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    push_report(LEVEL_INTERNAL, NULL, NOWHERE, NULL, formatv(fmt, args));
    va_end(args);
}

reportid_t warnf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    reportid_t id = push_report(LEVEL_WARNING, NULL, NOWHERE, NULL, formatv(fmt, args));
    va_end(args);
    return id;
}

reportid_t reportf(level_t level, node_t *node, const char *fmt, ...) {
    if (node == NULL) {
        assert("reporting a NULL node");
        va_list args;
        va_start(args, fmt);
        reportid_t id = push_report(level, NULL, NOWHERE, NULL, formatv(fmt, args));
        va_end(args);
        return id;
    }

    va_list args;
    va_start(args, fmt);
    reportid_t id = push_report(level, node->scanner, node->where, node, formatv(fmt, args));
    va_end(args);
    
    return id;
}

reportid_t reportv(level_t level, scanner_t *source, where_t where, const char *fmt, va_list args) {
    return push_report(level, source, where, NULL, formatv(fmt, args));
}

reportid_t report(level_t level, scanner_t *source, where_t where, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    reportid_t id = push_report(level, source, where, NULL, formatv(fmt, args));
    va_end(args);
    return id;
}

void report_underline(reportid_t id, const char *msg) {
    if (id != INVALID_REPORT)
        reports[id].underline = msg;
}

void report_note(reportid_t id, const char *msg) {
    if (id != INVALID_REPORT)
        reports[id].note = msg;
}

void report_tag(reportid_t id, const void *tag) {
    if (id != INVALID_REPORT)
        reports[id].tag = tag;
}

bool verbose = false;

void logfmt(const char *fmt, ...) {
    if (!verbose)
        return;

    printf(COLOUR_GREEN "log: " COLOUR_RESET);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

int report_code(void) {
    if (internal) {
        return 99;
    }

    return num_reports != 0;
}
