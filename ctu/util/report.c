#include "report.h"

#include "ctu/ast/scan.h"

#include "util.h"
#include "str.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

bool verbose = false;

static part_t *part_new(char *message, const node_t *node) {
    part_t *part = ctu_malloc(sizeof(part_t));
    part->message = message;
    part->node = node;
    return part;
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

static void report_scanner(const node_t *node) {
    const scan_t *scan = node->scan;
    where_t where = node->where;

    const char *path = scan->path;
    const char *language = scan->language;
    line_t line = where.first_line + 1;
    column_t column = where.first_column;

    fprintf(stderr, " => %s source [%s:%ld:%ld]\n",
        language, path, line, column
    );
}

static void report_header(message_t *message) {
    const char *lvl = report_level(message->level);

    fprintf(stderr, "%s: %s\n", lvl, message->message);

    if (message->node) {
        report_scanner(message->node);
    }
}

static char *padding(size_t len) {
    char *str = malloc(len + 1);
    memset(str, ' ', len);
    str[len] = '\0';
    return str;
}

static char *extract_line(const scan_t *scan, line_t line) {
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
        if (c == '\r' || c == '\n') {
            break;
        }
    }

    /** 
     * while windows line endings might technically be more correct
     * it doesnt make them any less painful to handle
     */
    char *str = malloc(len + 1);
    char *out = str;
    for (size_t i = 0; i < len - 1; i++) {
        char c = text[start + i];
        if (c == '\r') {
            continue;
        }
        *out++ = c;
    }
    *out = '\0';

    return str;
}

static char *build_underline(char *source, where_t where, char *note) {
    column_t front = where.first_column;
    column_t back = where.last_column;

    if (where.first_line < where.last_line) {
        back = strlen(source);
    }

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

static size_t longest_line(const scan_t *scan, line_t init, vector_t *parts) {
    char *num = format(" %ld ", init);
    size_t len = strlen(num);
    ctu_free(num);

    for (size_t i = 0; i < vector_len(parts); i++) {
        part_t *part = vector_get(parts, i);

        if (part->node->scan != scan) {
            continue;
        }

        line_t line = part->node->where.first_line + 1;
        char *it = format(" %ld ", line);
        len = MAX(len, strlen(it));
        ctu_free(it);
    }

    return len;
}

static char *right_align(line_t line, size_t width) {
    return format("%*ld", width, line);
}

static void report_source(message_t *message) {
    const node_t *node = message->node;
    if (!node) {
        return;
    }

    const scan_t *scan = node->scan;
    where_t where = node->where;

    line_t start = where.first_line;

    char *source = extract_line(scan, start);
    char *underline = build_underline(source, where, message->underline);

    size_t longest = longest_line(scan, start + 1, message->parts);
    char *line = right_align(start + 1, longest);
    char *pad = padding(longest);

    fprintf(stderr, "%s|\n", pad);
    fprintf(stderr, "%s| %s\n", line, source);
    fprintf(stderr, "%s| " COLOUR_PURPLE "%s\n" COLOUR_RESET, pad, underline);
}

static void report_part(message_t *message, part_t *part) {
    char *msg = part->message;

    const node_t *node = part->node;
    const scan_t *scan = node->scan;
    where_t where = node->where;

    line_t start = where.first_line;
    column_t front = where.first_column;

    char *source = extract_line(scan, start);
    char *underline = build_underline(source, where, msg);

    size_t longest = longest_line(scan, start + 1, message->parts);
    char *pad = padding(longest);
    char *line = right_align(start + 1, longest);

    if (message->node->scan != scan) {
        report_scanner(part->node);
    }

    fprintf(stderr, "%s> %s source %s:%ld:%ld\n", pad, 
        scan->language, scan->path,
        start + 1, front
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

reports_t *begin_reports() {
    reports_t *r = ctu_malloc(sizeof(reports_t));
    r->messages = vector_new(32);
    return r;
}

int end_reports(reports_t *reports, size_t total, const char *name) {
    size_t internal = 0;
    size_t fatal = 0;
    int result = 0;

    size_t errors = vector_len(reports->messages);

    for (size_t i = 0; i < errors; i++) {
        message_t *message = vector_get(reports->messages, i);
        switch (message->level) {
        case INTERNAL: 
            internal += 1;
            break;
        case ERROR:
            fatal += 1;
            break;
        default:
            break;
        }

        if (i >= total) {
            continue;
        }

        report_send(message);
    }

    if (internal > 0) {
        fprintf(stderr, "%zu internal error(s) encountered during %s stage\n", internal, name);
        result = 99;
    } else if (fatal > 0) {
        fprintf(stderr, "%zu fatal error(s) encountered during %s stage\n", fatal, name);
        result = 1;
    }

    return result;
}

static message_t *report_push(reports_t *reports,
                              level_t level,
                              const node_t *node, 
                              const char *fmt, 
                              va_list args)
{
    char *str = formatv(fmt, args);
    message_t *message = ctu_malloc(sizeof(message_t));

    message->level = level;
    message->parts = vector_new(1);
    message->message = str;
    message->underline = NULL;
    
    message->node = node;

    message->note = NULL;

    vector_push(&reports->messages, message);
    return message;
}

message_t *assert2(reports_t *reports, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    message_t *message = report_push(reports, INTERNAL, NULL, fmt, args);
    va_end(args);

    return message;
}

message_t *report2(reports_t *reports, level_t level, const node_t *node, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    message_t *msg = report_push(reports, level, node, fmt, args);

    va_end(args);

    return msg;
}

void report_append2(message_t *message, const node_t *node, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *str = formatv(fmt, args);
    va_end(args);

    vector_push(&message->parts, part_new(str, node));
}

void report_underline(message_t *message, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *msg = formatv(fmt, args);
    va_end(args);

    message->underline = msg;
}

void report_note2(message_t *message, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *msg = formatv(fmt, args);
    va_end(args);

    message->note = msg;
}

void logverbose(const char *fmt, ...) {
    if (!verbose) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s: %s\n", report_level(NOTE), formatv(fmt, args));
    va_end(args);
}
