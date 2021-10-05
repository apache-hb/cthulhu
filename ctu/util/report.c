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

static bool is_multiline_report(where_t where) {
    return where.last_line > where.first_line;
}

static size_t total_lines(where_t where) {
    return where.last_line - where.first_line;
}

static char *format_location(const scan_t *scan, where_t where) {
    if (is_multiline_report(where)) {
        return format("%s source [%s:%ld:%ld-%ld:%ld]",
            scan->language, scan->path, 
            where.first_line + 1, where.first_column,
            where.last_line + 1, where.last_column
        );
    } else {
        return format("%s source [%s:%ld:%ld]",
            scan->language, scan->path, 
            where.first_line + 1, where.first_column
        );
    }
}

static void report_scanner(const node_t *node) {
    const scan_t *scan = node->scan;
    where_t where = node->where;
    fprintf(stderr, " => %s\n", format_location(scan, where));
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
    text_t source = scan->source;
    while (start < source.size && line > 0) {
        char c = source.text[start++];
        if (c == '\n') {
            line -= 1;
        }
    }
    
    size_t len = 0;
    while (source.size > start + len) {
        char c = source.text[start + len++];
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
        char c = source.text[start + i];
        if (c == '\r') {
            continue;
        }
        *out++ = c;
    }
    *out = '\0';

    return nstrnorm(str, len - 1);
}

static char *build_underline(char *source, where_t where, char *note) {
    column_t front = where.first_column;
    column_t back = where.last_column;

    if (where.first_line < where.last_line) {
        back = strlen(source);
    }

    if (front >= back) {
        front = back;
    }

    size_t width = MAX(back - front, 1);
    size_t len = note ? strlen(note) : 0;
    char *str = malloc(back + width + len + 2);

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

/* stupid code but quick and simple */
static size_t base10_length(line_t digit) {
    if (digit < 10) { return 1; } 
    else if (digit < 100) { return 2; } 
    else if (digit < 1000) { return 3; } 
    else if (digit < 10000) { return 4; } 
    else if (digit < 100000) { return 5; } 
    else if (digit < 1000000) { return 6; }
    else if (digit < 10000000) { return 7; }
    else if (digit < 100000000) { return 8; }
    else if (digit < 1000000000) { return 9; }
    else if (digit < 10000000000) { return 10; }
    else if (digit < 100000000000) { return 11; }
    else if (digit < 1000000000000) { return 12; }
    else if (digit < 10000000000000) { return 13; }
    else if (digit < 100000000000000) { return 14; }
    else if (digit < 1000000000000000) { return 15; }
    else if (digit < 10000000000000000) { return 16; }
    else { return 17; }
}

static size_t longest_line(const scan_t *scan, line_t init, vector_t *parts) {
    size_t len = base10_length(init);

    for (size_t i = 0; i < vector_len(parts); i++) {
        part_t *part = vector_get(parts, i);

        if (part->node->scan != scan) {
            continue;
        }

        len = MAX(len, base10_length(part->node->where.first_line + 1));
    }

    return len;
}

static char *right_align(line_t line, int width) {
    return format("%*ld", width, line);
}

static char *underline_single(const scan_t *scan, where_t where) {
    
}

static char *underline_medium(const scan_t *scan, where_t where) {

}

static char *underline_large(const scan_t *scan, where_t where) {
    
}

static char *underline_source(const scan_t *scan, where_t where) {
    switch (total_lines(where)) {
    case 0: return underline_single(scan, where);
    case 1: case 2: return underline_medium(scan, where);
    default: return underline_large(scan, where);
    }
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

    ctu_free(line, strlen(line) + 1);
    ctu_free(source, strlen(source) + 1);
    ctu_free(pad, strlen(pad) + 1);
}

static void report_part(message_t *message, part_t *part) {
    char *msg = part->message;

    const node_t *node = part->node;
    const scan_t *scan = node->scan;
    where_t where = node->where;

    line_t start = where.first_line;

    char *source = extract_line(scan, start);
    char *underline = build_underline(source, where, msg);

    size_t longest = longest_line(scan, start + 1, message->parts);
    char *pad = padding(longest);
    char *line = right_align(start + 1, longest);

    if (message->node->scan != scan) {
        report_scanner(part->node);
    }

    char *loc = format_location(scan, where);

    fprintf(stderr, "%s> %s\n", pad, loc);
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

static void delete_message(message_t *message) {
    for (size_t i = 0; i < vector_len(message->parts); i++) {
        part_t *part = vector_get(message->parts, i);
        ctu_free(part->message, strlen(part->message) + 1);
        ctu_free(part, sizeof(part_t));
    }
    vector_delete(message->parts);

    ctu_free(message->message, strlen(message->message) + 1);
    if (message->underline) {
        ctu_free(message->underline, strlen(message->underline) + 1);
    }

    if (message->note) {
        ctu_free(message->note, strlen(message->note) + 1);
    }

    ctu_free(message, sizeof(message_t));
}

reports_t *begin_reports(void) {
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

void delete_reports(reports_t *reports) {
    for (size_t i = 0; i < vector_len(reports->messages); i++) {
        message_t *message = vector_get(reports->messages, i);
        delete_message(message);
    }
    vector_delete(reports->messages);
    ctu_free(reports, sizeof(reports_t));
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

message_t *ctu_assert(reports_t *reports, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    message_t *message = report_push(reports, INTERNAL, NULL, fmt, args);
    va_end(args);

    return message;
}

message_t *report(reports_t *reports, level_t level, const node_t *node, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    message_t *msg = report_push(reports, level, node, fmt, args);

    va_end(args);

    return msg;
}

void report_append(message_t *message, const node_t *node, const char *fmt, ...) {
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

void report_note(message_t *message, const char *fmt, ...) {
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
