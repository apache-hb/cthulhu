#include "scan.h"

#include "ctu/ast/compile.h"
#include "ctu/type/type.h"

#include <ctype.h>

typedef enum {
    TOK_KEYWORD,
    TOK_IDENTIFIER,
    TOK_INT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_STRING_LITERAL,
    TOK_EOF
} tok_type_t;

typedef enum {
    KEY_NONE = 0,

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

    KEY_ADD,
    KEY_SUB,
    KEY_DIV,
    KEY_MUL,
    KEY_REM,

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
} c_digit_t;

typedef struct {
    tok_type_t kind;

    /* location of the token */
    where_t where;

    union {
        key_t key; /* a keyword or punctuator */
        char *ident; /* an identifier */
        char *string; /* an escaped string literal */
        c_digit_t digit; /* either a float or an int */
    };
} tok_t;

typedef struct {
    reports_t *reports;
    c_data_t *context;
    scan_t *scanner;
    where_t where;
    size_t offset;
    char lookahead;
    tok_t tok;
} c_scan_t; 

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

static map_t *keywords = NULL;

static void init_keywords(void) {
    if (keywords != NULL) {
        return;
    }

    keywords = map_new(16);

    map_set(keywords, "auto", (void *) KEY_AUTO);
    map_set(keywords, "break", (void *) KEY_BREAK);
    map_set(keywords, "case", (void *) KEY_CASE);
    map_set(keywords, "char", (void *) KEY_CHAR);
    map_set(keywords, "const", (void *) KEY_CONST);
    map_set(keywords, "continue", (void *) KEY_CONTINUE);
    map_set(keywords, "default", (void *) KEY_DEFAULT);
    map_set(keywords, "do", (void *) KEY_DO);
    map_set(keywords, "double", (void *) KEY_DOUBLE);
    map_set(keywords, "else", (void *) KEY_ELSE);
    map_set(keywords, "enum", (void *) KEY_ENUM);
    map_set(keywords, "extern", (void *) KEY_EXTERN);
    map_set(keywords, "float", (void *) KEY_FLOAT);
    map_set(keywords, "for", (void *) KEY_FOR);
    map_set(keywords, "goto", (void *) KEY_GOTO);
    map_set(keywords, "if", (void *) KEY_IF);
    map_set(keywords, "inline", (void *) KEY_INLINE);
    map_set(keywords, "int", (void *) KEY_INT);
    map_set(keywords, "long", (void *) KEY_LONG);
    map_set(keywords, "register", (void *) KEY_REGISTER);
    map_set(keywords, "restrict", (void *) KEY_RESTRICT);
    map_set(keywords, "return", (void *) KEY_RETURN);
    map_set(keywords, "short", (void *) KEY_SHORT);
    map_set(keywords, "signed", (void *) KEY_SIGNED);
    map_set(keywords, "sizeof", (void *) KEY_SIZEOF);
    map_set(keywords, "static", (void *) KEY_STATIC);
    map_set(keywords, "struct", (void *) KEY_STRUCT);
    map_set(keywords, "switch", (void *) KEY_SWITCH);
    map_set(keywords, "typedef", (void *) KEY_TYPEDEF);
    map_set(keywords, "union", (void *) KEY_UNION);
    map_set(keywords, "unsigned", (void *) KEY_UNSIGNED);
    map_set(keywords, "void", (void *) KEY_VOID);
    map_set(keywords, "volatile", (void *) KEY_VOLATILE);
    map_set(keywords, "while", (void *) KEY_WHILE);
    
    map_set(keywords, "_Alignas", (void *) KEY_ALIGNAS);
    map_set(keywords, "_Alignof", (void *) KEY_ALIGNOF);
    map_set(keywords, "_Atomic", (void *) KEY_ATOMIC);
    map_set(keywords, "_Bool", (void *) KEY_BOOL);
    map_set(keywords, "_Complex", (void *) KEY_COMPLEX);
    map_set(keywords, "_Generic", (void *) KEY_GENERIC);
    map_set(keywords, "_Decimal128", (void *) KEY_DECIMAL128);
    map_set(keywords, "_Decimal32", (void *) KEY_DECIMAL32);
    map_set(keywords, "_Decimal64", (void *) KEY_DECIMAL64);
    map_set(keywords, "_Imaginary", (void *) KEY_IMAGINARY);
    map_set(keywords, "_Noreturn", (void *) KEY_NORETURN);
    map_set(keywords, "_Thread_local", (void *) KEY_THREAD_LOCAL);
    map_set(keywords, "_Static_assert", (void *) KEY_STATIC_ASSERT);
}

static tok_t build_tok(c_scan_t *scan, tok_type_t kind) {
    tok_t tok = { 
        .kind = kind,
        .where = scan->where
    };
    return tok;
}

static tok_t build_ident(c_scan_t *scan, char *ident) {
    key_t key = (key_t)map_get(keywords, ident);
    if (key != KEY_NONE) {
        tok_t tok = build_tok(scan, TOK_KEYWORD);
        tok.key = key;
        return tok;
    }

    tok_t tok = build_tok(scan, TOK_IDENTIFIER);
    tok.ident = ident;
    return tok;
}

static char skip_whitespace(c_scan_t *scan) {
    char c = next_char(scan);
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        c = next_char(scan);
    }
    return c;
}

static bool isident1(char c) {
    return isalpha(c) || c == '_';
}

static bool isident2(char c) {
    return isalnum(c) || c == '_';
}

static tok_t get_tok(c_scan_t *scan) {
    scan->where.first_line = scan->where.last_line;
    scan->where.first_column = scan->where.last_column;

    char c = skip_whitespace(scan);

    size_t start = scan->offset - 1;

    if (c == '\0') {
        return build_tok(scan, TOK_EOF);
    } else if (isident1(c)) {
        size_t end = start;
        while (isident2(peek_char(scan))) {
            end = scan->offset;
            c = next_char(scan);
        }
        char *ident = ctu_memdup(scan->scanner->text + start, end - start + 1);
        ident[end - start] = '\0';
        return build_ident(scan, ident);
    }

    report2(scan->reports, ERROR,
        node_new(scan->scanner, scan->where),
        "unknown character %c", c
    );

    return get_tok(scan);
}

static tok_t next_tok(c_scan_t *scan) {
    tok_t tok = scan->tok;
    if (tok.kind != TOK_EOF) {
        scan->tok = get_tok(scan);
    }
    return tok;
}


static tok_t peek_tok(c_scan_t *scan) {
    return scan->tok;
}

static bool is_key(tok_t tok, key_t key) {
    return tok.kind == TOK_KEYWORD 
        && tok.key == key;
}

static type_t *parse_typespec(c_scan_t *scan) {
    tok_t tok = next_tok(scan);

    type_t *result = NULL;

    if (is_key(tok, KEY_CONST)) {
        result = parse_typespec(scan);
        /* make const */
    } else if (is_key(tok, KEY_VOLATILE)) {
        result = parse_typespec(scan);
        /* make volatile */
    } else if (is_key(tok, KEY_UNSIGNED)) {
        
    }

    while (true) {
        tok = peek_tok(scan);
        if (is_key(tok, KEY_MUL)) {
            next_tok(scan);
            result = type_ptr(result);
        } else {
            break;
        }
    }

    return result;
}

static bool parse_decl(c_scan_t *scan) {
    tok_t tok = next_tok(scan);
    (void)tok;
    (void)parse_typespec;
    return scan != NULL;
}

static c_t *parse_program(c_scan_t *scan) {
    /* parse-decl-list */
    bool ok = true;
    while (ok) {
        ok = parse_decl(scan);
    }
    return NULL;
}

c_t *c_compile(reports_t *reports, file_t *fd) {
    init_keywords();

    where_t start = { 0, 0, 0, 0 };

    c_scan_t scan = {
        .reports = reports,
        .context = c_data_new(),
        .scanner = scan_file(reports, "C99", fd),
        .where = start,
        .offset = 0
    };

    scan.lookahead = get_char(&scan);
    scan.tok = get_tok(&scan);

    (void)node_of;

    return parse_program(&scan);
}
