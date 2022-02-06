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

static void add_top_level(sema_t *sema, size_t tag, const char *name, hlir_t *hlir) {
    hlir_t *other = sema_get(sema, tag, name);
    if (other != NULL) {
        message_t *id = report(sema->reports, ERROR, hlir->node, "redefinition of %s", name);
        report_append(id, other->node, "previously defined here");
        return;
    }

    sema_set(sema, tag, name, hlir);
}

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

static type_t *parse_type_specifier(sema_t *sema, c11_token_t tok) {
    bool is_extern = false;
    if (tok_is_key(tok, KEY_EXTERN)) {
        is_extern = true;
        tok = next_token(sema);
    }

    bool is_const = false;
    if (tok_is_key(tok, KEY_CONST)) {
        is_const = true;
        tok = next_token(sema);
    }

    if (tok_is_key(tok, KEY_VOID)) {
        return VOID;
    }

    return NULL;
}

static bool parse_decl(sema_t *sema, c11_token_t tok) {
    type_t *type = parse_type_specifier(sema, tok);

    while (true) {
        type_t *inner = parse_trailing_type_parts(sema, type);
        ident_result_t name = get_ident(sema);
        if (name.ident == NULL) {
            report(sema->reports, ERROR, name.tok.node, "expected identifier");
            return false;
        }

        hlir_t *decl = hlir_new_value(tok.node, name.ident, inner);
        hlir_build_value(decl, NULL);
        add_top_level(sema, TAG_VARS, name.ident, decl);

        if (eat_key(sema, KEY_SEMICOLON, NULL)) {
            return true;
        }

        if (eat_key(sema, KEY_COMMA, NULL)) {
            continue;
        }

        report(sema->reports, ERROR, tok.node, "expected ';' or ','");
        return false;
    }
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

    return NULL;
}
