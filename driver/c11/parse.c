#include "parse.h"

#include "cthulhu/hlir/sema.h"

typedef enum {
    TAG_STRUCTS,
    TAG_ENUMS,
    TAG_FUNCS,
    TAG_TYPEDEFS,
    TAG_VARS,
    
    TAG_MAX
} c11_tag_t;

static size_t TOPLEVEL_SIZES[TAG_MAX] = {
    [TAG_STRUCTS] = MAP_MASSIVE,
    [TAG_ENUMS] = MAP_MASSIVE,
    [TAG_FUNCS] = MAP_MASSIVE,
    [TAG_TYPEDEFS] = MAP_MASSIVE,
    [TAG_VARS] = MAP_MASSIVE
};

static size_t BODY_SIZES[TAG_MAX] = {
    [TAG_STRUCTS] = MAP_SMALL,
    [TAG_ENUMS] = MAP_SMALL,
    [TAG_FUNCS] = MAP_SMALL,
    [TAG_TYPEDEFS] = MAP_SMALL,
    [TAG_VARS] = MAP_SMALL
};

static type_t *VOID = NULL;

void c11_init_types(void) {
    VOID = type_void("void");
}

static c11_token_t next_token(sema_t *sema) {
    c11_lexer_t *lex = sema_get_data(sema);
    return c11_lexer_next(lex);
}

static c11_token_t peek_token(sema_t *sema) {
    c11_lexer_t *lex = sema_get_data(sema);
    return c11_lexer_peek(lex);
}

static bool eat_key(sema_t *sema, c11_keyword_t key, c11_token_t *out) {
    c11_token_t token = peek_token(sema);
    if (out != NULL) {
        *out = token;
    }
    
    if (!tok_is_key(token, key)) {
        return false;
    }

    next_token(sema);
    return true;
}

typedef struct {
    char *ident;
    c11_token_t tok;
} ident_result_t;

static ident_result_t get_ident(sema_t *sema) {
    c11_token_t token = peek_token(sema);
    ident_result_t result;
    result.tok = token;
    if (!tok_is_ident(token)) {
        result.ident = NULL;
        return result;
    }

    next_token(sema);
    result.ident = token.ident;
    return result;
}

static type_t *parse_trailing_type_parts(sema_t *sema, type_t *inner) {
    c11_token_t loc;
    while (eat_key(sema, KEY_STAR, &loc)) {
        inner = type_pointer(NULL, inner, loc.node);
    }
    return inner;
}

static type_t *parse_type_specifier(sema_t *sema, c11_token_t front) {
    if (tok_is_key(front, KEY_VOID)) {
        return parse_trailing_type_parts(sema, VOID);
    }

    return NULL;
}

static bool parse_decl(sema_t *sema, c11_token_t tok) {
    type_t *type = parse_type_specifier(sema, tok);
    if (type == NULL) {
        report(sema->reports, ERROR, tok.node, "expected type specifier");
        return false;
    }

    ident_result_t result = get_ident(sema);
    if (result.ident == NULL) {
        report(sema->reports, ERROR, result.tok.node, "expected declaration name");
        return false;
    }

    return true;
}

static bool c11_parse_decl(sema_t *sema) {
    c11_token_t tok = next_token(sema);
    
    /* technically valid */
    if (tok_is_key(tok, KEY_SEMICOLON)) {
        return true;
    }

    return parse_decl(sema, tok);
}

hlir_t *c11_compile(reports_t *reports, scan_t *scan) {
    c11_lexer_t *lex = c11_lexer_new(scan);

    sema_t *sema = sema_new(NULL, reports, TAG_MAX, TOPLEVEL_SIZES);
    sema_set_data(sema, lex);

    while (c11_parse_decl(sema)) {
        // empty
    }

    while (true) {
        c11_token_t tok = c11_lexer_next(lex);
        message_t *id = report(reports, NOTE, tok.node, "token: %d", tok.type);
        if (tok.type == TOK_IDENT) {
            report_underline(id, "id: %s", tok.ident);
        } else if (tok.type == TOK_INVALID) {
            report_underline(id, "error: %s", tok.error);
        }
        if (tok.type == TOK_EOF || tok.type == TOK_INVALID) {
            break;
        }
    }

    return NULL;
}
