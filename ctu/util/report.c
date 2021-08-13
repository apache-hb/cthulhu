#include "report.h"

#include "ctu/ast/scan.h"

#include "util.h"
#include "str.h"

#include <stdarg.h>
#include <stdlib.h>

typedef struct {
    /* the level of this error */
    level_t level;

    /* error message displayed at the top */
    char *message;

    /* source and location, if scan is NULL then location is ignored */
    scan_t *scan;
    where_t location;
} message_t;

/* has an internal error happened */
/* we track this to exit(99) for fuzzing reasons */
static bool internal = false;

/* the number of error reports to store */
static size_t reports = 0;
static size_t used = 0;
static message_t *messages = NULL;

static const char *report_level(level_t level) {
    switch (level) {
    case INTERNAL: return COLOUR_CYAN "ice" COLOUR_RESET;
    case ERROR: return COLOUR_RED "error" COLOUR_RESET;
    case WARNING: return COLOUR_YELLOW "warning" COLOUR_RESET;
    case NOTE: return COLOUR_GREEN "note" COLOUR_RESET;

    default: 
        return COLOUR_PURPLE "unknown" COLOUR_RESET;
    }
}

static void report_header(message_t *message) {
    const char *lvl = report_level(message->level);

    fprintf(stderr, "%s: %s\n", lvl, message->message);

    if (message->scan) {
        where_t location = message->location;

        const char *path = message->scan->path;
        line_t line = location.first_line;
        column_t column = location.first_column;

        fprintf(stderr, " => [%s:%ld:%ld]\n",
            path, line, column
        );
    }
}

static void report_send(message_t *message) {
    report_header(message);
}

void begin_report(size_t limit) {
    reports = limit;
    messages = ctu_malloc(sizeof(message_t) * limit);
}

void end_report(const char *name) {
    for (size_t i = 0; i < used; i++) {
        message_t *message = &messages[i];
        report_send(message);
    }
    
    if (internal) {
        fprintf(stderr, "exiting during %s due to a fatal error", name);
        exit(99);
    }
}

static report_t report_add(level_t level, const char *fmt, va_list args) {
    if (level == INTERNAL) {
        internal = true;
    }

    if (used >= reports) {
        return INVALID_REPORT;
    }

    char *str = formatv(fmt, args);
    message_t msg = {
        .level = level,
        .message = str,
        .scan = NULL,
        .location = nowhere
    };

    messages[used] = msg;
    return used++;
}

void assert(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    report_add(INTERNAL, fmt, args);
    va_end(args);
}

report_t report(level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    report_t id = report_add(level, fmt, args);
    va_end(args);
    return id;
}
