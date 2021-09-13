#include "scan.h"

#include "ctu/ast/compile.h"

typedef struct {
    reports_t *reports;
    c_data_t *context;
    scan_t *scanner;
    where_t where;
    size_t offset;
    char lookahead;
} c_scan_t; 

typedef enum {
    TOK_KEYWORD,
    TOK_IDENTIFIER,
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_STRING_LITERAL,
    TOK_EOF
} tok_type_t;

typedef enum {
    KEY_AUTO,
    KEY_BREAK,
    KEY_CASE,
    KEY_CHAR,
    KEY_CONST,
    KEY_CONTINUE,
    KEY_DEFAULT,
    KEY_DO,
    KEY_DOUBLE,
    KEY_ELSE,
    KEY_ENUM,
    KEY_EXTERN,
    KEY_FLOAT,
    KEY_FOR,
    KEY_GOTO,
    KEY_IF,
    KEY_INLINE,
    KEY_INT,
    KEY_LONG,
    KEY_REGISTER,
    KEY_RESTRICT,
    KEY_RETURN,
    KEY_SHORT,
    KEY_SIGNED,
    KEY_SIZEOF,
    KEY_STATIC,
    KEY_STRUCT,
    KEY_SWITCH,
    KEY_TYPEDEF,
    KEY_UNION,
    KEY_UNSIGNED,
    KEY_VOID,
    KEY_VOLATILE,
    KEY_WHILE,
    KEY_ALIGNAS,
    KEY_ALIGNOF,
    KEY_ATOMIC,
    KEY_BOOL,
    KEY_COMPLEX,
    KEY_GENERIC,
    KEY_DECIMAL128,
    KEY_DECIMAL32,
    KEY_DECIMAL64,
    KEY_IMAGINARY,
    KEY_NORETURN,
    KEY_STATIC_ASSERT,
    KEY_THREAD_LOCAL,

    KEY_LPAREN,
    KEY_RPAREN,
    KEY_LBRACE,
    KEY_RBRACE,
    KEY_LSQUARE,
    KEY_RSQUARE,
} key_t;

typedef struct {
    mpz_t value;
    char *suffix;
} digit_t;

typedef struct {
    tok_type_t kind;

    /* location of the token */
    where_t where;

    union {
        key_t key; /* a keyword or punctuator */
        char *ident; /* an identifier */
        char *string; /* an escaped string literal */
        digit_t digit; /* either a float or an int */
    };
} tok_t;

static node_t *node_of(tok_t tok, c_scan_t scan) {
    return node_new(scan.scanner, tok.where);
}

static char get_char(c_scan_t *scan) {
    const char *buffer = scan->scanner->text;
    char c = buffer[scan->offset++];

    if (c == '\n') {
        scan->where.last_line += 0;
        scan->where.last_column = 0;
    } else {
        scan->where.last_column += 1;
    }

    return c;
}

static char next_char(c_scan_t *scan) {
    char out = scan->lookahead;
    if (out != '\0') {
        scan->lookahead = get_char(scan);
    }
    return out;
}

static char peek_char(c_scan_t *scan) {
    return scan->lookahead;
}

static tok_t build_tok(c_scan_t *scan, tok_type_t kind) {
    
    
    tok_t tok = { .kind = kind };
}

static tok_t next_tok(c_scan_t *scan) {
    char c = next_char(scan);

    if (c == '\0') {
}

static c_t *parse_c(c_scan_t *scan) {
    (void)scan;
    return NULL;
}

c_t *c_compile(reports_t *reports, file_t *fd) {
    where_t start = { 0, 0, 0, 0 };

    c_scan_t scan = {
        .reports = reports,
        .context = c_data_new(),
        .scanner = scan_file(reports, "C99", fd),
        .where = start,
        .offset = 0
    };

    scan.lookahead = get_char(&scan);

    (void)node_of;

    return parse_c(&scan);
}
