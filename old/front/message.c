#include "compile.h"

#include <string.h>
#include <inttypes.h>

#include "cthulhu/util/report.h"

typedef struct {
    const char *msg;
    const char *note;

    node_t *source;

    scanner_t *scanner;
    YYLTYPE loc;

    /* true if error, false if warning */
    bool fatal;
} error_t;

static size_t max_errs = 20;
static size_t err_idx = 0;
static error_t *errors = NULL;

static bool error_already_reported(size_t idx) {
    node_t *node = errors[idx].source;

    if (!node)
        return false;
    
    for (size_t i = 0; i < idx; i++)
        if (errors[i].source == node)
            return true;

    return false;
}

void max_errors(size_t num) {
    max_errs = num;
    errors = malloc(sizeof(error_t) * num);
}

int yyerror(YYLTYPE *yylloc, void *scanner, scanner_t *x, const char *msg) {
    (void)scanner;

    add_lexer_error(strdup(msg), x, *yylloc);

    return 1;
}

static msg_idx_t add_err(const char *msg, node_t *source, scanner_t *scanner, YYLTYPE loc, bool fatal) {
    error_t err = { msg, NULL, source, scanner, loc, fatal };

    if (err_idx <= max_errs) {
        errors[err_idx++] = err;
    }

    if (fatal) {
        add_fail();
    }

    return err_idx - 1;
}

msg_idx_t add_lexer_error(const char *msg, scanner_t *scanner, YYLTYPE loc) {
    return add_err(msg, NULL, scanner, loc, true);
}

msg_idx_t add_error(const char *msg, node_t *node) {
    return add_err(msg, node, node->source, node->loc, true);
}

msg_idx_t add_warn(const char *msg, node_t *node) {
    return add_err(msg, node, node->source, node->loc, false);
}

void add_note(msg_idx_t id, const char *note) {
    errors[id].note = note;
}

static size_t get_start(scanner_t *scanner, loc_t first_line) {
    size_t distance = 0;
    
    while (first_line > 0) {
        char c = scanner->text[distance];
        if (!c) {
            return distance;
        }

        if (c == '\n') {
            first_line -= 1;
        }

        distance += 1;
    }

    return distance;
}

static void print_padding(size_t padding) {
    for (size_t i = 0; i < padding; i++) {
        printf(" ");
    }
}

static size_t underline_location(scanner_t *scanner, YYLTYPE loc) {
    char *line_str = format("%" PRId64, loc.first_line);
    size_t line_len = strlen(line_str);

    size_t padding = line_len + 2;

    /* print padding above source */
    print_padding(padding);
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
    print_padding(padding);
    printf("| ");

    loc_t col = 0;
    loc_t len = 1;

    if (loc.first_line == loc.last_line) {
        len = loc.last_column - loc.first_column;
    }

    loc_t first_col = loc.first_column - 1;

    printf(COLOUR_PURPLE);
    /* print underline */
    while (true) {
        c = scanner->text[start++];
        if (!c || c == '\n') {
            break;
        } else {
            if (col >= first_col) {
                printf(col == first_col ? "^" : "~");
            } else {
                printf("%s", c == '\t' ? "\t" : " ");
            }
            len--;
            col++;
        }
    }
    printf(COLOUR_RESET "\n");

    return padding;
}

bool write_messages(const char *stage) {
    size_t fatal = 0;

    for (size_t i = 0; i < err_idx; i++) {
        if (error_already_reported(i))
            continue;

        error_t err = errors[i];

        if (err.fatal) {
            reportf(err.msg);
            fatal += 1;
        } else {
            warnf(err.msg);
        }

        fprintf(stderr, " => [%s:%" PRId64 ":%" PRId64 "]\n", 
            err.scanner->path, err.loc.first_line, err.loc.first_column
        );
        size_t padding = underline_location(err.scanner, err.loc);

        if (err.note) {
            print_padding(padding);
            printf("= " COLOUR_CYAN "note: " COLOUR_RESET "%s\n", err.note);
        }
    }

    if (fatal) {
        reportf("%s: aborting due to %llu error(s)", stage, fatal);
        exit(1);
    }

    return fatal;
}
