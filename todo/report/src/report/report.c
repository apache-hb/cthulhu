#include "report/report.h"

#include "core/macros.h"
#include "memory/memory.h"
#include "base/panic.h"

#include "std/str.h"
#include "std/vector.h"

#include "scan/node.h"

#include "stacktrace/stacktrace.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

bool verbose = false;

static bool is_valid_node(const node_t *node)
{
    return !node_is_builtin(node);
}

static part_t *part_new(char *message, const node_t *node)
{
    part_t *part = MEM_ALLOC(sizeof(part_t), "part", NULL);
    part->message = message;
    part->node = node;
    return part;
}

typedef struct
{
    const char *plain;
    const char *coloured;
} level_format_t;

static level_format_t kFormats[eLevelTotal] = {
    [eSorry] = { "sorry", ANSI_MAGENTA "sorry" ANSI_RESET },
    [eInternal] = {"internal", ANSI_CYAN "ice" ANSI_RESET},
    [eFatal] = {"error", ANSI_RED "error" ANSI_RESET},
    [eWarn] = {"warning", ANSI_YELLOW "warning" ANSI_RESET},
    [eInfo] = {"note", ANSI_GREEN "note" ANSI_RESET},
};

static const char *report_level_str(level_t level)
{
    return kFormats[level].plain;
}

static const char *report_level(level_t level)
{
    return kFormats[level].coloured;
}

static bool is_multiline_report(where_t where)
{
    return where.last_line > where.first_line;
}

static size_t total_lines(where_t where)
{
    return where.last_line - where.first_line;
}

static char *format_location(const char *base, const scan_t *scan, where_t where)
{
    const char *language = scan_language(scan);
    const char *path = scan_path(scan);
    if (is_multiline_report(where))
    {
        return format("%s source [%s:%" PRI_LINE ":%" PRI_COLUMN "-%" PRI_LINE ":%" PRI_COLUMN "]", language, path + strlen(base), where.first_line + 1,
                      where.first_column, where.last_line + 1, where.last_column);
    }

    return format("%s source [%s:%" PRI_LINE ":%" PRI_COLUMN "]", language, path + strlen(base), where.first_line + 1, where.first_column);
}

static void report_scanner(const char *base, const node_t *node)
{
    const scan_t *scan = node_get_scan(node);
    where_t where = node_get_location(node);
    fprintf(stderr, " => %s\n", format_location(base, scan, where));
}

static void report_header(const char *base, message_t *message)
{
    const char *lvl = report_level(message->level);

    fprintf(stderr, "%s: %s\n", lvl, message->message);

    if (is_valid_node(message->node))
    {
        report_scanner(base, message->node);
    }
}

static char *padding(size_t len)
{
    char *str = MEM_ALLOC(len + 1, "padding", NULL);
    memset(str, ' ', len);
    str[len] = '\0';
    return str;
}

static char *extract_line(const scan_t *scan, line_t line)
{
    size_t start = 0;
    text_view_t source = scan_source(scan);
    while (start < source.size && line > 0)
    {
        char c = source.text[start++];
        if (c == '\n')
        {
            line -= 1;
        }
        if (c == '\0')
        {
            break;
        }
    }

    size_t len = 0;
    while (source.size > start + len)
    {
        char c = source.text[start + len];
        if (c == '\r' || c == '\n' || c == '\0')
        {
            break;
        }
        len += 1;
    }

    /**
     * while windows line endings might technically be more correct
     * it doesnt make them any less painful to handle
     */
    CTASSERT(len != SIZE_MAX); // verify against overflow
    char *str = MEM_ALLOC(len + 1, "extract_line", NULL);
    char *out = str;
    for (size_t i = 0; i < len; i++)
    {
        char c = source.text[start + i];
        if (c == '\r')
        {
            continue;
        }

        if (c == '\0')
        {
            break;
        }

        *out++ = c;
    }
    *out = '\0';

    char *norm = str_normalizen(str, (size_t)(out - str));

    ctu_free(str);

    return norm;
}

static bool safe_isspace(int c)
{
    switch (c)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return true;

    default:
        return false;
    }
}

static char *build_underline(const char *source, where_t where, const char *note)
{
    column_t front = where.first_column;
    column_t back = where.last_column;

    size_t limit = strlen(source);

    if (where.first_line < where.last_line)
    {
        back = limit;
    }

    if (front > back)
    {
        front = back;
    }

    size_t width = MAX(back - front, 1);

    size_t len = note ? strlen(note) : 0;

    // allocate space for the underline
    // +1 for the space
    // +1 for the null terminator
    char *str = MEM_ALLOC(back + width + len + 2, "underline", NULL);

    column_t idx = 0;

    /* use correct tabs or spaces when underlining */
    while (front > idx && idx < limit)
    {
        char c = source[idx];
        str[idx++] = safe_isspace(c) ? c : ' ';
    }

    str[idx] = '^';
    memset(str + idx + 1, '~', width - 1);
    str[idx + width] = ' ';

    if (note)
    {
        memcpy(str + idx + width + 1, note, len);
        str[idx + width + len + 1] = '\0';
    }
    else
    {
        str[idx + width + 1] = '\0';
    }

    return str;
}

#define MAX_BASE10_LEN 32 // 32 digits is enough for a 64 bit integer and its null terminator

static int base10_length(line_t num)
{
    int width = 0;

    while (num > 0)
    {
        num /= 10;
        width++;
    }

    return width;
}

static size_t longest_line(const scan_t *scan, line_t init, vector_t *parts)
{
    int len = base10_length(init);

    for (size_t i = 0; i < vector_len(parts); i++)
    {
        part_t *part = vector_get(parts, i);

        const scan_t *self = node_get_scan(part->node);

        if (self != scan)
        {
            continue;
        }

        where_t where = node_get_location(part->node);
        len = MAX(len, base10_length(where.first_line + 1));
    }

    return len;
}

static char *right_align(line_t line, int width)
{
    return format("%*" PRI_LINE, width, line);
}

/**
 * formats a source span for a single line
 *
 *      |
 *  line| source text
 *      | ^~~~~~ underline message
 */
static char *format_single(const node_t *node, const char *underline)
{
    where_t where = node_get_location(node);
    const scan_t *scan = node_get_scan(node);

    line_t first_line = where.first_line + 1;
    int align = base10_length(first_line);

    char *pad = padding(align);
    char *digit = right_align(first_line, align);

    char *firstLineOfSource = extract_line(scan, where.first_line);

    return format(" %s|\n"
                  " %s| %s\n"
                  " %s|" ANSI_MAGENTA " %s\n" ANSI_RESET,
                  pad, digit, firstLineOfSource, pad, build_underline(firstLineOfSource, where, underline));
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
static char *format_medium2(const node_t *node, const char *underline)
{
    where_t where = node_get_location(node);
    const scan_t *scan = node_get_scan(node);

    line_t first_line = where.first_line + 1;
    int align = base10_length(first_line);

    char *pad = padding(align);
    char *digit = right_align(first_line, align);

    char *firstLineOfSource = extract_line(scan, where.first_line);
    char *lastLineOfSource = extract_line(scan, where.last_line);

    return format(" %s|\n"
                  " %s>" ANSI_MAGENTA " %s\n" ANSI_RESET " %s>" ANSI_MAGENTA " %s\n" ANSI_RESET
                  " %s|" ANSI_MAGENTA " %s\n" ANSI_RESET,
                  pad, digit, firstLineOfSource, pad, lastLineOfSource, pad,
                  build_underline(lastLineOfSource, where, underline));
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
static char *format_medium3(const node_t *node, const char *underline)
{
    where_t where = node_get_location(node);
    const scan_t *scan = node_get_scan(node);

    line_t first_line = where.first_line + 1;
    int align = base10_length(first_line);

    char *pad = padding(align);
    char *digit = right_align(first_line, align);

    char *firstLineOfSource = extract_line(scan, where.first_line);
    char *secondLineOfSource = extract_line(scan, where.first_line + 1);
    char *lastLineOfSource = extract_line(scan, where.last_line);

    return format(" %s|\n"
                  " %s>" ANSI_MAGENTA " %s\n" ANSI_RESET " %s>" ANSI_MAGENTA " %s\n" ANSI_RESET
                  " %s>" ANSI_MAGENTA " %s\n" ANSI_RESET " %s|" ANSI_MAGENTA " %s\n" ANSI_RESET,
                  pad, digit, firstLineOfSource, pad, secondLineOfSource, pad, lastLineOfSource, pad,
                  build_underline(lastLineOfSource, where, underline));
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
static char *format_large(const node_t *node, const char *underline)
{
    where_t where = node_get_location(node);
    const scan_t *scan = node_get_scan(node);

    line_t first_line = where.first_line + 1;
    line_t last_line = where.last_line + 1;
    int align = MAX(base10_length(first_line), base10_length(last_line)) + 1;

    char *pad = padding(align);
    char *alignedFirstDigit = right_align(first_line, align);
    char *alignedLastDigit = right_align(last_line, align);

    char *firstSourceLine = extract_line(scan, where.first_line);
    char *lastSourceLine = extract_line(scan, where.last_line);

    return format(" %s|\n"
                  " %s>" ANSI_MAGENTA " %s\n" ANSI_RESET " %s>" ANSI_MAGENTA " ...\n" ANSI_RESET
                  " %s>" ANSI_MAGENTA " %s\n" ANSI_RESET " %s|" ANSI_MAGENTA " %s\n" ANSI_RESET,
                  pad, alignedFirstDigit, firstSourceLine, pad, alignedLastDigit, lastSourceLine, pad,
                  build_underline(lastSourceLine, where, underline));
}

static char *format_source(const node_t *node, const char *underline)
{
    switch (total_lines(node_get_location(node)))
    {
    case 0:
        return format_single(node, underline);
    case 1:
        return format_medium2(node, underline);
    case 2:
        return format_medium3(node, underline);
    default:
        return format_large(node, underline);
    }
}

static void report_source(message_t *message)
{
    const node_t *node = message->node;
    if (!is_valid_node(node))
    {
        return;
    }

    fprintf(stderr, "%s", format_source(node, message->underline));
}

static void report_part(const char *base, message_t *message, part_t *part)
{
    char *msg = part->message;

    const node_t *node = part->node;
    const scan_t *scan = node_get_scan(node);
    where_t where = node_get_location(node);

    line_t start = where.first_line;

    size_t longest = longest_line(scan, start + 1, message->parts);
    char *pad = padding(longest);

    if (node_get_scan(message->node) != scan)
    {
        report_scanner(base, part->node);
    }

    if (is_valid_node(message->node))
    {
        char *loc = format_location(base, scan, where);
        fprintf(stderr, "%s> %s\n", pad, loc);
        fprintf(stderr, "%s", format_source(message->node, msg));
    }
}

static void send_note(const char *note)
{
    const char *level = report_level_str(eInfo);
    size_t len = strlen(level);

    // +2 for the `: ` in the final print
    char *padding = format("\n%s", str_repeat(" ", len + 2));

    char *aligned = str_replace(note, "\n", padding);

    fprintf(stderr, "%s: %s\n", report_level(eInfo), aligned);
}

static bool report_send(const char *base, message_t *message)
{
    report_header(base, message);
    report_source(message);

    for (size_t i = 0; i < vector_len(message->parts); i++)
    {
        report_part(base, message, vector_get(message->parts, i));
    }

    if (message->note)
    {
        send_note(message->note);
    }

    return message->level <= eFatal;
}

USE_DECL
reports_t *begin_reports(void)
{
    reports_t *reports = MEM_ALLOC(sizeof(reports_t), "reports", NULL);
    reports->messages = vector_new(32);
    return reports;
}

static const char *paths_base(vector_t *messages)
{
    size_t len = vector_len(messages);
    vector_t *result = vector_new(len);

    for (size_t i = 0; i < len; i++)
    {
        message_t *message = vector_get(messages, i);
        if (is_valid_node(message->node))
        {
            const scan_t *scan = node_get_scan(message->node);
            vector_push(&result, (char *)scan_path(scan));
        }

        vector_t *parts = message->parts;
        for (size_t j = 0; j < vector_len(parts); j++)
        {
            part_t *part = vector_get(parts, j);
            if (is_valid_node(part->node))
            {
                const scan_t *scan = node_get_scan(message->node);
                vector_push(&result, (char *)scan_path(scan));
            }
        }
    }

    if (vector_len(result) <= 1)
    {
        return "";
    }

    return str_common_prefix(result);
}

USE_DECL
status_t end_reports(reports_t *reports, const char *name, report_config_t settings)
{
    CTASSERT(reports != NULL);
    CTASSERT(name != NULL);

    size_t total = settings.limit - 1;

    size_t internal = 0;
    size_t fatal = 0;

    size_t fatalMessagesSuppressed = 0;
    size_t warningsSuppressed = 0;

    status_t result = EXIT_OK;

    size_t errors = vector_len(reports->messages);
    const char *common = paths_base(reports->messages);

    for (size_t i = 0; i < errors; i++)
    {
        message_t *message = vector_get(reports->messages, i);

        if (settings.warningsAreErrors && message->level == eWarn)
        {
            message->level = eFatal;
        }

        switch (message->level)
        {
        case eInternal:
            internal += 1;
            break;
        case eFatal:
            fatal += 1;
            break;
        default:
            break;
        }

        if (i > total && message->level != eInternal)
        {
            switch (message->level)
            {
            case eFatal:
                fatalMessagesSuppressed += 1;
                break;
            case eWarn:
                warningsSuppressed += 1;
                break;
            default:
                break;
            }
            continue;
        }

        report_send(common, message);
    }

    if (internal > 0)
    {
        fprintf(stderr, "%zu internal error(s) encountered during %s stage\n", internal, name);
        result = EXIT_INTERNAL;
    }
    else if (fatal > 0)
    {
        fprintf(stderr, "%zu fatal error(s) encountered during %s stage\n", fatal, name);
        result = EXIT_ERROR;
    }

    if (warningsSuppressed > 0 || fatalMessagesSuppressed > 0)
    {
        fprintf(stderr, "%zu extra warning(s) and %zu extra error(s) suppressed\n", warningsSuppressed,
                fatalMessagesSuppressed);
    }

    vector_reset(reports->messages);

    return result;
}

static message_t *report_push(reports_t *reports, level_t level, const node_t *node, const char *fmt, va_list args)
{
    CTASSERT(reports != NULL);

    char *str = vformat(fmt, args);
    message_t *message = MEM_ALLOC(sizeof(message_t), "message", reports);

    message->level = level;
    message->parts = vector_new(1);
    message->message = str;
    message->underline = NULL;

    message->node = node;

    message->note = NULL;

    if (level == eInternal)
    {
        report_send("internal error", message);
        bt_print_trace(stderr);
    }
    else
    {
        vector_push(&reports->messages, message);
    }
    return message;
}

USE_DECL
message_t *report(reports_t *reports, level_t level, const node_t *node, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    message_t *msg = report_push(reports, level, node, fmt, args);

    va_end(args);

    return msg;
}

USE_DECL
void report_append(message_t *message, const node_t *node, const char *fmt, ...)
{
    CTASSERT(message != NULL);

    va_list args;
    va_start(args, fmt);
    char *str = vformat(fmt, args);
    va_end(args);

    vector_push(&message->parts, part_new(str, node));
}

USE_DECL
void report_underline(message_t *message, const char *fmt, ...)
{
    CTASSERT(message != NULL);

    va_list args;
    va_start(args, fmt);
    char *msg = vformat(fmt, args);
    va_end(args);

    message->underline = msg;
}

USE_DECL
void report_note(message_t *message, const char *fmt, ...)
{
    CTASSERT(message != NULL);

    va_list args;
    va_start(args, fmt);
    char *msg = vformat(fmt, args);
    va_end(args);

    message->note = msg;
}

USE_DECL
void logverbose(const char *fmt, ...)
{
    if (!verbose)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s: %s\n", report_level(eInfo), vformat(fmt, args));
    va_end(args);

    fflush(stderr);
}