#include "report.h"

#include "ctu/ast/scan.h"

#include "util.h"
#include "str.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *message;
    scan_t *scan;
    where_t where;
} part_t;

typedef struct {
    /* the level of this error */
    level_t level;

    /* error message displayed at the top */
    char *message;
    char *underline;

    vector_t *parts;

    /* source and location, if scan is NULL then location is ignored */
    scan_t *scan;
    where_t where;
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
        where_t where = message->where;

        const char *path = message->scan->path;
        const char *language = message->scan->language;
        line_t line = where.first_line;
        column_t column = where.first_column;

        fprintf(stderr, " => %s source [%s:%ld:%ld]\n",
            language, path, line, column
        );
    }
}

static char *padding(size_t len) {
    char *str = malloc(len + 1);
    memset(str, ' ', len);
    str[len] = '\0';
    return str;
}

static char *extract_line(scan_t *scan, line_t line) {
    size_t start = 0;
    const char *text = scan->text;
    while (text[start] && line > 0) {
        char c = text[start++];
        if (c == '\n') {
            line -= 1;
        }
    }
    
    size_t len = 0;
    while (text[start + len]) {
        char c = text[start + len++];
        if (c == '\n') {
            break;
        }
    }

    char *str = malloc(len + 1);
    memcpy(str, text + start, len - 1);
    str[len] = '\0';
    return str;
}

static char *build_underline(column_t front, column_t back, char *note) {
    size_t width = back - front;
    size_t len = note ? strlen(note) : 0;
    char *str = malloc(back + len + 1);

    memset(str, ' ', front);
    memset(str + front, '^', width);
    
    if (note) {
        memcpy(str + front + width, note, len);
        str[front + width + len] = '\0';
    } else {
        str[front + width] = '\0';
    }

    return str;
}

static void report_source(message_t *message) {
    scan_t *scan = message->scan;
    where_t where = message->where;
    if (!scan) {
        return;
    }

    line_t start = where.first_line;

    column_t front = where.first_column;
    column_t back = where.last_column;

    char *line = format(" %ld ",start);
    size_t len = strlen(line);
    char *pad = padding(len);

    fprintf(stderr, "%s|\n", pad);
    fprintf(stderr, "%s| %s\n", line, extract_line(scan, start));
    fprintf(stderr, "%s| " COLOUR_PURPLE "%s\n" COLOUR_RESET, pad, build_underline(front, back, message->underline));
}

static bool report_send(message_t *message) {
    report_header(message);
    report_source(message);

    return message->level <= ERROR;
}

void begin_report(size_t limit) {
    reports = limit;
    messages = ctu_malloc(sizeof(message_t) * limit);
}

void end_report(const char *name) {
    bool fatal = false;

    for (size_t i = 0; i < used; i++) {
        message_t *message = get_message(i);
        fatal = fatal || report_send(message);
    }
    
    if (internal) {
        fprintf(stderr, "exiting during %s due to an internal error", name);
        exit(99);
    }

    if (fatal) {
        fprintf(stderr, "exiting during %s due to a fatal error\n", name);
        exit(1);
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
        .parts = vector_new(1),
        .message = str,
        .scan = scan,
        .where = where
    };

    messages[used] = msg;
    return used++;
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
