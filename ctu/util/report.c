#include "report.h"

#include "ctu/ast/scan.h"

#include "util.h"
#include "str.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *message;
    scan_t *scan;
    where_t where;
} part_t;

static part_t *part_new(char *message, scan_t *scan, where_t where) {
    part_t *part = ctu_malloc(sizeof(part_t));
    part->message = message;
    part->scan = scan;
    part->where = where;
    return part;
}

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

    /* extra note */
    char *note;
} message_t;

/* has an internal error happened */
/* we track this to exit(99) for fuzzing reasons */
static bool internal = false;
static bool fatal = false;
static bool self = false;

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

static void report_scanner(scan_t *scan, where_t where) {
    const char *path = scan->path;
    const char *language = scan->language;
    line_t line = where.first_line;
    column_t column = where.first_column;

    fprintf(stderr, " => %s source [%s:%ld:%ld]\n",
        language, path, line, column
    );
}

static void report_header(message_t *message) {
    const char *lvl = report_level(message->level);

    fprintf(stderr, "%s: %s\n", lvl, message->message);

    if (message->scan) {
        report_scanner(message->scan, message->where);
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
    while (text[start] != '\0' && line > 0) {
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

static char *build_underline(char *source, column_t front, column_t back, char *note) {
    size_t width = back - front;
    size_t len = note ? strlen(note) : 0;
    char *str = malloc(back + len + 2);

    column_t idx = 0;

    /* use correct tabs or spaces when underlining */
    while (front > idx) {
        char c = source[idx];
        str[idx++] = isspace(c) ? c : ' ';
    }

    str[idx] = '^';
    memset(str + idx + 1, '~', width - 1);
    str[idx + width] = ' ';
    if (note) {
        memcpy(str + idx + width + 1, note, len);
        str[idx + width + len + 1] = '\0';
    } else {
        str[idx + width + 1] = '\0';
    }

    return str;
}

static size_t longest_line(scan_t *scan, where_t where, vector_t *parts) {
    char *fmt = format(" %ld ", where.first_line);

    size_t len = strlen(fmt);

    free(fmt);

    for (size_t i = 0; i < vector_len(parts); i++) {
        part_t *part = vector_get(parts, i);

        if (part->scan != scan) {
            continue;
        }

        line_t line = part->where.first_line;
        char *it = format(" %ld ", line);
        len = MAX(len, strlen(it));
        free(it);
    }

    return len;
}

static char *right_align(line_t line, size_t width) {
    char *num = format("%ld", line);
    return format("%*s", width, num);
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

    char *source = extract_line(scan, start);
    char *underline = build_underline(source, front, back, message->underline);

    size_t longest = longest_line(scan, where, message->parts);
    char *line = right_align(start, longest);
    char *pad = padding(longest);

    fprintf(stderr, "%s|\n", pad);
    fprintf(stderr, "%s| %s\n", line, source);
    fprintf(stderr, "%s| " COLOUR_PURPLE "%s\n" COLOUR_RESET, pad, underline);
}

static void report_part(message_t *message, part_t *part) {
    char *msg = part->message;
    scan_t *scan = part->scan;
    where_t where = part->where;

    size_t start = where.first_line;
    column_t front = where.first_column;
    column_t back = where.last_column;

    char *source = extract_line(scan, start);
    char *underline = build_underline(source, front, back, msg);

    size_t longest = longest_line(scan, part->where, message->parts);
    char *pad = padding(longest);
    char *line = right_align(start, longest);

    if (message->scan != scan) {
        report_scanner(part->scan, part->where);
    }

    fprintf(stderr, "%s> %s source %s:%ld:%ld\n", pad, 
        scan->language, scan->path, 
        where.first_line, where.first_column
    );
    fprintf(stderr, "%s|\n", pad);
    fprintf(stderr, "%s| %s\n", line, source);
    fprintf(stderr, "%s| " COLOUR_PURPLE "%s\n" COLOUR_RESET, pad, underline);
}

static void send_note(const char *note) {
    fprintf(stderr, "%s: %s\n", report_level(NOTE), note);
}

static bool report_send(message_t *message) {
    report_header(message);
    report_source(message);

    for (size_t i = 0; i < vector_len(message->parts); i++) {
        report_part(message, vector_get(message->parts, i));
    }

    if (message->note) {
        send_note(message->note);
    }

    return message->level <= ERROR;
}

void begin_report(size_t limit) {
    reports = limit;
    messages = ctu_malloc(sizeof(message_t) * limit);
}

static void message_delete(message_t *message) {
    for (size_t i = 0; i < vector_len(message->parts); i++) {
        part_t *part = vector_get(message->parts, i);
        ctu_free(part->message);
    }

    vector_delete(message->parts);

    if (message->message) {
        ctu_free(message->message);
    }

    if (message->underline) {
        ctu_free(message->underline);
    }

    if (message->note) {
        ctu_free(message->note);
    }
}

void end_report(bool quit, const char *name) {
    for (size_t i = 0; i < used; i++) {
        message_t *message = get_message(i);
        if (report_send(message)) {
            self = true;
            fatal = true;
        }
        message_delete(message);
    }
    
    if (internal) {
        fprintf(stderr, "exiting during %s due to an internal error\n", name);
        exit(99);
    }

    if (self && !quit) {
        fprintf(stderr, "fatal error in %s\n", name);
    }

    if (fatal && quit) {
        fprintf(stderr, "fatal error in %s, exiting\n", name);
        exit(1);
    }

    self = false;
    used = 0;
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
        .where = where,
        .note = NULL
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

void report_append(report_t id, scan_t *scan, where_t where, const char *fmt, ...) {
    if (id == INVALID_REPORT) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    char *msg = formatv(fmt, args);
    va_end(args);

    message_t *message = get_message(id);
    vector_push(&message->parts, part_new(msg, scan, where));
}

void report_note(report_t id, const char *fmt, ...) {
    if (id == INVALID_REPORT) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    char *msg = formatv(fmt, args);
    va_end(args);

    message_t *message = get_message(id);
    message->note = msg;
}
