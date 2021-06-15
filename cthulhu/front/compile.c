#include "compile.h"

#include <stdarg.h>
#include <limits.h>

#include "bison.h"
#include "flex.h"

#include "cthulhu/util/report.h"

typedef struct {
    size_t cursor;
    const char *text;
} string_buffer_t;

static int flex_file_next(void *it) {
    FILE *file = it;
    int letter = fgetc(file);
    return letter == EOF ? 0 : letter;
}

static int flex_string_next(void *it) {
    string_buffer_t *buffer = it;

    int letter = buffer->text[buffer->cursor];
    if (letter)
        buffer->cursor += 1;

    return letter;
}

#define BUF_SIZE 0x1000

static scanner_t *make_scanner(const char *path, void *data, int(*next)(void*)) {
    char *text = malloc(BUF_SIZE);

    scanner_t *scanner = malloc(sizeof(scanner_t));
    
    scanner->path = path;
    scanner->ast = NULL;
    scanner->data = data;
    scanner->next = next;
    scanner->text = text;
    scanner->len = 0;
    scanner->size = BUF_SIZE;

    return scanner;
}

nodes_t *compile_file(const char *path, FILE *stream) {
    int err;
    yyscan_t scan;
    scanner_t *extra = make_scanner(path, stream, flex_file_next);

    if ((err = yylex_init_extra(extra, &scan))) {
        reportf("yylex_init_extra -> %d", err);
        return NULL;
    }

    yyset_in(stream, scan);

    if ((err = yyparse(scan, extra))) {
        return NULL;
    }

    yylex_destroy(scan);

    return extra->ast;
}

nodes_t *compile_string(const char *path, const char *text) {
    int err;
    yyscan_t scan;
    string_buffer_t strbuf = { 0, text };
    scanner_t *extra = make_scanner(path, &strbuf, flex_string_next);
    YY_BUFFER_STATE buffer;

    if ((err = yylex_init_extra(extra, &scan))) {
        reportf("yylex_init_extra -> %d", err);
        return NULL;
    }

    if (!(buffer = yy_scan_string(text, scan))) {
        reportf("yy_scan_string -> NULL");
        return NULL;
    }

    if ((err = yyparse(scan, extra))) {
        yy_delete_buffer(buffer, scan);
        return NULL;
    }

    yylex_destroy(scan);

    return extra->ast;
}

void flex_init(YYLTYPE *loc) {
    loc->distance = 0;
    loc->first_line = 1;
    loc->last_line = 1;
    loc->first_column = 1;
    loc->last_column = 1;
}

typedef struct {
    char *msg;
    size_t distance;
    int64_t line, col, end_col;
    scanner_t *source;
} error_t;

static size_t max_errs = 20;
static size_t err_idx = 0;
static error_t *errors = NULL;

void max_errors(size_t num) {
    max_errs = num;
    errors = malloc(sizeof(error_t) * num);
}

int yyerror(YYLTYPE *yylloc, void *scanner, scanner_t *x, const char *msg) {
    (void)scanner;

    scan_reportf(yylloc, x, msg);

    return 1;
}

#define IS_NEWLINE(l) (l == '\n' || l == '\r')

static void print_line(scanner_t *x, int64_t line, int64_t col, int64_t end_col, size_t dist) {
    /* find the begining of this line */
    size_t begin = dist;
    char letter = x->text[begin];
    while (begin && !IS_NEWLINE(letter)) {
        letter = x->text[begin--];
    }

    /* find the end of this line */
    size_t end = dist;
    letter = x->text[end];
    while (letter && !IS_NEWLINE(letter)) {
        letter = x->text[end++];
    }

    if (!letter) {
        char buffer;
        YYLTYPE loc;
        while (flex_provide(&loc, x, &buffer)) { 
            if (IS_NEWLINE(buffer))
                break;
            end += 1;
        }
    }

    char *num = format("%" PRId64 " ", line);
    size_t len = strlen(num);

    for (size_t i = 0; i < len; i++) {
        printf(" ");
    }
    printf("|\n");

    printf("%s| ", num);

    /* print the line */
    for (size_t i = begin; i < end; i++) {
        printf("%c", x->text[i]);
    }
    printf("\n");

    for (size_t i = 0; i < len; i++) {
        printf(" ");
    }
    printf("| ");

    for (int64_t i = 0; i < col - 1; i++) {
        printf(" ");
    }
    printf(COLOUR_CYAN);
    int64_t dots = end_col - col;
    while (dots--)
        printf("^");
    printf(COLOUR_RESET "\n");
}

void write_errors(void) {
    for (size_t i = 0; i < err_idx; i++) {
        error_t err = errors[i];
        
        reportf(err.msg);
        fprintf(stderr, " => [%s:%" PRId64 ":%" PRId64 "]\n", 
            err.source->path, err.line, err.col
        );
        print_line(err.source, err.line, err.col, err.end_col, err.distance);
    }
}

void scan_reportf(YYLTYPE *where, scanner_t *x, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *msg = formatv(fmt, args);
    va_end(args);

    error_t err = {
        msg, where->distance,
        where->first_line, where->first_column,
        where->last_column, x
    };

    if (err_idx < max_errs) {
        errors[err_idx++] = err;
    }

    add_fail();
}

static void add_char(scanner_t *dst, int c) {
    if (c > CHAR_MAX || c < CHAR_MIN) {
        reportf("add_char(%d) out of range", c);
    }

    ENSURE_SIZE(dst->text, dst->len, dst->size, sizeof(char), BUF_SIZE)
    dst->text[dst->len++] = (char)c;
    dst->text[dst->len] = 0;
}

int flex_provide(YYLTYPE *loc, scanner_t *x, char *buf) {
    int letter = x->next(x->data);
    buf[0] = letter;
    if (letter) {
        loc->distance += 1;
        add_char(x, letter);
    }
    return !!letter;
}
