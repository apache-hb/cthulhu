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

    /* associated notes */
    vector_t *notes;
} message_t;

/* has an internal error happened */
/* we track this to exit(99) for fuzzing reasons */
static bool internal = false;

/* the number of error reports to store */
static size_t reports = 0;
static size_t used = 0;
static message_t *messages = NULL;

static message_t *get_message(size_t index) {
    return &messages[index];
}

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
        const char *language = message->scan->language;
        line_t line = location.first_line;
        column_t column = location.first_column;

        fprintf(stderr, " => %s source [%s:%ld:%ld]\n",
            language, path, line, column
        );
    }
}

static void report_note(const char *note) {
    fprintf(stderr, COLOUR_GREEN "note: " COLOUR_RESET "%s\n", note);
}

static void report_notes(message_t *message) {
    vector_t *notes = message->notes;
    size_t len = vector_len(notes);
    for (size_t i = 0; i < len; i++) {
        const char *note = vector_get(notes, i);
        report_note(note);
    }
}

static void report_send(message_t *message) {
    report_header(message);
    report_notes(message);
}

void begin_report(size_t limit) {
    reports = limit;
    messages = ctu_malloc(sizeof(message_t) * limit);
}

void end_report(const char *name) {
    for (size_t i = 0; i < used; i++) {
        message_t *message = get_message(i);
        report_send(message);
    }
    
    if (internal) {
        fprintf(stderr, "exiting during %s due to a fatal error", name);
        exit(99);
    }
}

static report_t report_add(
    level_t level, 
    scan_t *scan, 
    where_t where, 
    const char *fmt, 
    va_list args
) {
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
        .scan = scan,
        .location = where,
        .notes = vector_new(0)
    };

    messages[used] = msg;
    return used++;
}

static void note_add(report_t id, const char *fmt, va_list args) {
    char *msg = formatv(fmt, args);
    message_t *message = get_message(id);
    vector_push(&message->notes, msg);
}

void assert(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    report_add(INTERNAL, NULL, nowhere, fmt, args);
    va_end(args);
}

report_t report(level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    report_t id = report_add(level, NULL, nowhere, fmt, args);
    va_end(args);
    return id;
}

report_t reportf(level_t level, scan_t *scan, where_t where, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    report_t id = report_add(level, scan, where, fmt, args);
    va_end(args);
    return id;
}

void add_note(report_t id, const char *fmt, ...) {
    if (id == INVALID_REPORT) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    note_add(id, fmt, args);
    va_end(args);
}
