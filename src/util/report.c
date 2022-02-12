#include "cthulhu/util/report.h"
#include "cthulhu/util/util.h"
#include "cthulhu/util/str.h"

#include "cthulhu/ast/scan.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

bool verbose = false;

static part_t *part_new(char *message, const node_t *node) {
    part_t *part = ctu_malloc(sizeof(part_t));
    part->message = message;
    part->node = node;
    return part;
}

typedef struct {
    const char *plain;
    const char *coloured;
} level_format_t;

static level_format_t FORMATS[LEVEL_TOTAL] = {
    [INTERNAL] = { "internal", COLOUR_CYAN "ice" COLOUR_RESET },
    [ERROR] = { "error", COLOUR_RED "error" COLOUR_RESET },
    [WARNING] = { "warning", COLOUR_YELLOW "warning" COLOUR_RESET },
    [NOTE] = { "note", COLOUR_GREEN "note" COLOUR_RESET },
};

static const char *report_level_str(level_t level) {
    return FORMATS[level].plain;
}

static const char *report_level(level_t level) {
    return FORMATS[level].coloured;
}

static bool is_multiline_report(where_t where) {
    return where.last_line > where.first_line;
}

static size_t total_lines(where_t where) {
    return where.last_line - where.first_line;
}

static char *format_location(const char *base, const scan_t *scan, where_t where) {
    if (is_multiline_report(where)) {
        return format("%s source [%s:%ld:%ld-%ld:%ld]",
            scan->language, scan->path + strlen(base), 
            where.first_line + 1, where.first_column,
            where.last_line + 1, where.last_column
        );
    } else {
        return format("%s source [%s:%ld:%ld]",
            scan->language, scan->path + strlen(base), 
            where.first_line + 1, where.first_column
        );
    }
}

static void report_scanner(const char *base, const node_t *node) {
    const scan_t *scan = node->scan;
    where_t where = node->where;
    fprintf(stderr, " => %s\n", format_location(base, scan, where));
}

static void report_header(const char *base, message_t *message) {
    const char *lvl = report_level(message->level);

    fprintf(stderr, "%s: %s\n", lvl, message->message);

    if (message->node) {
        report_scanner(base, message->node);
    }
}

static char *padding(size_t len) {
    char *str = ctu_malloc(len + 1);
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
        if (c == '\0') {
            break;
        }
    }
    
    size_t len = 0;
    while (source.size > start + len) {
        char c = source.text[start + len];
        if (c == '\r' || c == '\n' || c == '\0') {
            break;
        }
        len += 1;
    }

    /** 
     * while windows line endings might technically be more correct
     * it doesnt make them any less painful to handle
     */
    char *str = ctu_malloc(len + 1);
    char *out = str;
    for (size_t i = 0; i < len; i++) {
        char c = source.text[start + i];
        if (c == '\r') {
            continue;
        }

        if (c == '\0') {
            break;
        }
        
        *out++ = c;
    }
    *out = '\0';

    return nstrnorm(str, (size_t)(out - str));
}

static char *build_underline(const char *source, where_t where, const char *note) {
    column_t front = where.first_column;
    column_t back = where.last_column;

    if (where.first_line < where.last_line) {
        back = strlen(source);
    }

    if (front > back) {
        front = back;
    }

    size_t width = MAX(back - front, 1);

    // allocate space for the underline
    // +1 for the space
    // +1 for the null terminator
    // char *padding = ctu_malloc(back + width + 1 + 1);

    size_t len = note ? strlen(note) : 0;

    char *str = ctu_malloc(back + width + len + 2);

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
static int base10_length(line_t digit) {
    return ceil(log10(digit)) + 1;
}

static size_t longest_line(const scan_t *scan, line_t init, vector_t *parts) {
    int len = base10_length(init);

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

/**
 * formats a source span for a single line
 * 
 *      |
 *  line| source text
 *      | ^~~~~~ underline message
 */
static char *format_single(const scan_t *scan, where_t where, const char *underline) {
    line_t first_line = where.first_line + 1;
    int align = base10_length(first_line);

    char *pad = padding(align);
    char *digit = right_align(first_line, align);

    char *first_source = extract_line(scan, where.first_line);

    return format(
        " %s|\n"
        " %s| %s\n"
        " %s|" COLOUR_PURPLE " %s\n" COLOUR_RESET,
        pad,
        digit, first_source,
        pad, build_underline(first_source, where, underline)
    );
}

/**
 * formats a source span for 2 lines
 * 
 *       |
 *  line1> source text 1
 *       > source text on the next line
 *       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ underline message
 *       |
 */
static char *format_medium2(const scan_t *scan, where_t where, const char *underline) {
    line_t first_line = where.first_line + 1;
    int align = base10_length(first_line);

    char *pad = padding(align);
    char *digit = right_align(first_line, align);

    char *first_source = extract_line(scan, where.first_line);
    char *last_source = extract_line(scan, where.last_line);

    return format(
        " %s|\n"
        " %s>" COLOUR_PURPLE " %s\n" COLOUR_RESET
        " %s>" COLOUR_PURPLE " %s\n" COLOUR_RESET
        " %s|" COLOUR_PURPLE " %s\n" COLOUR_RESET,
        pad,
        digit, first_source,
        pad, last_source,
        pad, build_underline(last_source, where, underline)
    );
}

/**
 * formats a source span for 3 lines
 * 
 *       |
 *  line1> source text 1
 *       > source text on the next line
 *       > source text on the third line
 *       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ underline message
 *       |
 */
static char *format_medium3(const scan_t *scan, where_t where, const char *underline) {
    line_t first_line = where.first_line + 1;
    int align = base10_length(first_line);

    char *pad = padding(align);
    char *digit = right_align(first_line, align);

    char *first_source = extract_line(scan, where.first_line);
    char *middle_source = extract_line(scan, where.first_line + 1);
    char *last_source = extract_line(scan, where.last_line);

    return format(
        " %s|\n"
        " %s>" COLOUR_PURPLE " %s\n" COLOUR_RESET
        " %s>" COLOUR_PURPLE " %s\n" COLOUR_RESET
        " %s>" COLOUR_PURPLE " %s\n" COLOUR_RESET
        " %s|" COLOUR_PURPLE " %s\n" COLOUR_RESET,
        pad,
        digit, first_source,
        pad, middle_source,
        pad, last_source,
        pad, build_underline(last_source, where, underline)
    );
}

/**
 * formats a source span for more than 3 lines
 * 
 *       |
 *  line1> source text 1
 *       > ...
 *  lineN> source text on the final line
 *       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ underline message
 *       |
 */
static char *format_large(const scan_t *scan, where_t where, const char *underline) {
    line_t first_line = where.first_line + 1;
    line_t last_line = where.last_line + 1;
    int align = MAX(base10_length(first_line), base10_length(last_line)) + 1;

    char *pad = padding(align);
    char *first_digit = right_align(first_line, align);
    char *last_digit = right_align(last_line, align);

    char *first_source = extract_line(scan, where.first_line);
    char *last_source = extract_line(scan, where.last_line);

    return format(
        " %s|\n"
        " %s>" COLOUR_PURPLE " %s\n" COLOUR_RESET
        " %s>" COLOUR_PURPLE " ...\n" COLOUR_RESET
        " %s>" COLOUR_PURPLE " %s\n" COLOUR_RESET
        " %s|" COLOUR_PURPLE " %s\n" COLOUR_RESET,
        pad,
        first_digit, first_source,
        pad,
        last_digit, last_source,
        pad, build_underline(last_source, where, underline)
    );
}

static char *format_source(const scan_t *scan, where_t where, const char *underline) {
    switch (total_lines(where)) {
    case 0: return format_single(scan, where, underline);
    case 1: return format_medium2(scan, where, underline);
    case 2: return format_medium3(scan, where, underline);
    default: return format_large(scan, where, underline);
    }
}

static void report_source(message_t *message) {
    const node_t *node = message->node;
    if (!node) {
        return;
    }

    const scan_t *scan = node->scan;
    where_t where = node->where;

    fprintf(stderr, "%s", format_source(scan, where, message->underline));
}

static void report_part(const char *base, message_t *message, part_t *part) {
    char *msg = part->message;

    const node_t *node = part->node;
    const scan_t *scan = node->scan;
    where_t where = node->where;

    line_t start = where.first_line;

    size_t longest = longest_line(scan, start + 1, message->parts);
    char *pad = padding(longest);

    if (message->node->scan != scan) {
        report_scanner(base, part->node);
    }

    char *loc = format_location(base, scan, where);

    fprintf(stderr, "%s> %s\n", pad, loc);
    fprintf(stderr, "%s", format_source(scan, where, msg));
}

static void send_note(const char *note) {
    const char *level = report_level_str(NOTE);
    size_t len = strlen(level);

    // +2 for the `: ` in the final print
    char *padding = format("\n%s", strmul(" ", len + 2));
    
    char *aligned = replacestr(note, "\n", padding);

    fprintf(stderr, "%s: %s\n", report_level(NOTE), aligned);
}

static bool report_send(const char *base, message_t *message) {
    report_header(base, message);
    report_source(message);

    for (size_t i = 0; i < vector_len(message->parts); i++) {
        report_part(base, message, vector_get(message->parts, i));
    }

    if (message->note) {
        send_note(message->note);
    }

    return message->level <= ERROR;
}

reports_t *begin_reports(void) {
    reports_t *reports = ctu_malloc(sizeof(reports_t));
    reports->messages = vector_new(32);
    return reports;
}

static const char *paths_base(vector_t *messages) {
    size_t len = vector_len(messages);
    vector_t *result = vector_new(len);

    for (size_t i = 0; i < len; i++) {
        message_t *message = vector_get(messages, i);
        if (message->node != NULL) {
            vector_push(&result, (char*)message->node->scan->path);
        }

        vector_t *parts = message->parts;
        for (size_t j = 0; j < vector_len(parts); j++) {
            part_t *part = vector_get(parts, j);
            if (part->node != NULL) {
                vector_push(&result, (char*)part->node->scan->path);
            }
        }
    }

    return vector_len(result) > 0 ? common_prefix(result) : "";
}

int end_reports(reports_t *reports, size_t total, const char *name) {
    size_t internal = 0;
    size_t fatal = 0;
    int result = 0;

    size_t errors = vector_len(reports->messages);
    const char *common = paths_base(reports->messages);

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

        report_send(common, message);
    }

    if (internal > 0) {
        fprintf(stderr, "%zu internal error(s) encountered during %s stage\n", internal, name);
        result = 99;
        printf("\a");
    } else if (fatal > 0) {
        fprintf(stderr, "%zu fatal error(s) encountered during %s stage\n", fatal, name);
        result = 1;
        printf("\a");
    }

    reports->messages = vector_new(0);

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