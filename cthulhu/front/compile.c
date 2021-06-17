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
    loc->first_line = 1;
    loc->last_line = 1;
    loc->first_column = 1;
    loc->last_column = 1;
}

typedef struct {
    const char *msg;
    scanner_t *scanner;
    YYLTYPE loc;
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

    add_error(strdup(msg), x, *yylloc);

    return 1;
}

void add_error(const char *msg, scanner_t *scanner, YYLTYPE loc) {
    error_t err = { msg, scanner, loc };

    if (err_idx <= max_errs) {
        errors[err_idx++] = err;
    }

    add_fail();
}

static size_t get_start(scanner_t *scanner, loc_t first_line) {
    size_t distance = 0;
    
    while (first_line > 0) {
        char c = scanner->text[distance];
        if (!c) {
            //reportf("get_start out of range");
            return distance;
        }

        if (c == '\n') {
            first_line -= 1;
        }

        distance += 1;
    }

    return distance;
}

static void underline_location(scanner_t *scanner, YYLTYPE loc) {
    char *line_str = format("%" PRId64, loc.first_line);
    size_t line_len = strlen(line_str);

    /* print padding above source */
    for (size_t i = 0; i < line_len + 2; i++) {
        printf(" ");
    }
    printf("|\n");

    /* print source line */
    printf(" %s | ", line_str);
    /* bison locations start at 1 rather than 0 */
    size_t start = get_start(scanner, loc.first_line - 1);
    size_t off = start;
    char c;
    while (true) {
        c = scanner->text[off++];
        if (!c || c == '\n') {
            break;
        }
        printf("%c", c);
    }

    /* print padding below source */
    printf("\n");
    for (size_t i = 0; i < line_len + 2; i++) {
        printf(" ");
    }
    printf("| ");

    loc_t col = 0;

    /* print underline */
    while (true) {
        c = scanner->text[start++];
        if (!c || c == '\n') {
            break;
        } else {
            if (col++ == loc.first_column - 1)
                printf("^");
            else
                printf("%s", c == '\t' ? "\t" : " ");
        }
    }
    printf("\n");
}

void write_errors(void) {
    for (size_t i = 0; i < err_idx; i++) {
        error_t err = errors[i];

        reportf(err.msg);
        fprintf(stderr, " => [%s:%" PRId64 ":%" PRId64 "]\n", 
            err.scanner->path, err.loc.first_line, err.loc.first_column
        );
        underline_location(err.scanner, err.loc);
    }
}

static void add_char(scanner_t *dst, int c) {
    if (c > CHAR_MAX || c < CHAR_MIN) {
        reportf("add_char(%d) out of range", c);
    }

    ENSURE_SIZE(dst->text, dst->len, dst->size, sizeof(char), BUF_SIZE)
    dst->text[dst->len++] = (char)c;
    dst->text[dst->len] = 0;
}

int flex_provide(scanner_t *x, char *buf) {
    int letter = x->next(x->data);
    buf[0] = letter;
    if (letter && letter != '\r') {
        add_char(x, letter);
    }
    return !!letter;
}
