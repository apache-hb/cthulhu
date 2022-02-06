#include "lex.h"

#include <ctype.h>

static map_t *c11_keywords = NULL;

static void add_keyword(const char *key, c11_keyword_t word) {
    map_set(c11_keywords, key, (void*)word);
}

void c11_keyword_init(void) {
    c11_keywords = map_new(MAP_BIG);

    add_keyword("void", KEY_VOID);

    add_keyword("char", KEY_CHAR);
    add_keyword("short", KEY_SHORT);
    add_keyword("int", KEY_INT);
    add_keyword("long", KEY_LONG);

    add_keyword("signed", KEY_SIGNED);
    add_keyword("unsigned", KEY_UNSIGNED);

    add_keyword("const", KEY_CONST);
    add_keyword("volatile", KEY_VOLATILE);

    add_keyword("extern", KEY_EXTERN);
}

static bool isident1(char c) {
    return isalpha(c) || c == '_';
}

static bool isident2(char c) {
    return isalnum(c) || c == '_';
}

static where_t NOWHERE = { 0, 0, 0, 0 };

static c11_token_t new_token(c11_token_type_t type, node_t *node) {
    c11_token_t token = {
        .type = type,
        .node = node
    };
    
    return token;
}

static c11_token_t finish_token(c11_lexer_state_t *state, c11_token_type_t type) {
    // fix the off by one error
    // theres probably a better way of doing this
    // but this works
    where_t where = state->where;
    where.first_column = where.first_column > 0 ? where.first_column - 1 : 0;
    
    node_t *node = node_new(state->scan, where);
    return new_token(type, node);
}

static c11_token_t finish_keyword(c11_lexer_state_t *state, c11_keyword_t keyword) {
    c11_token_t tok = finish_token(state, TOK_KEYWORD);
    tok.keyword = keyword;
    return tok;
}

static c11_token_t finish_error(c11_lexer_state_t *state, const char *error) {
    c11_token_t tok = finish_token(state, TOK_INVALID);
    tok.error = error;
    return tok;
}

static char get_char(c11_lexer_state_t *state) {
    text_t *text = &state->scan->source;

    if (text->size <= state->offset) {
        return '\0';
    }

    char c = '\r';
    while (c == '\r') {
        c = text->text[state->offset++];
    }

    return c;
}

static c11_lexer_state_t *c11_new_lexer_state(scan_t *scan) {
    c11_lexer_state_t *state = ctu_malloc(sizeof(c11_lexer_state_t));
    state->scan = scan;
    state->where = NOWHERE;
    state->offset = 0;
    state->ahead = get_char(state);
    return state;
}

static c11_token_t next_token(c11_lexer_state_t *state);

c11_lexer_t *c11_lexer_new(scan_t *scan) {
    c11_lexer_t *lexer = ctu_malloc(sizeof(c11_lexer_t));
    lexer->state = c11_new_lexer_state(scan);
    lexer->token = next_token(lexer->state);
    return lexer;
}

static char next_char(c11_lexer_state_t *state) {
    char c = state->ahead;

    if (c == '\n') {
        state->where.last_column = 0;
        state->where.last_line += 1;
    } else {
        state->where.last_column += 1;
    }

    state->ahead = get_char(state);
    return c;
}

static char peek_char(c11_lexer_state_t *state) {
    return state->ahead;
}

static void begin_span(c11_lexer_state_t *state) {
    state->where.first_line = state->where.last_line;
    state->where.first_column = state->where.last_column;
}

static char begin_token(c11_lexer_state_t *state) {
    char c = next_char(state);
    while (isspace(c)) {
        c = next_char(state);
    }

    begin_span(state);

    return c;
}

static char *state_substring(c11_lexer_state_t *state, size_t start, size_t end) {
    const char *text = state->scan->source.text + start - 1;
    return ctu_strndup(text, end - start + 1);
}

static c11_token_t finish_ident(c11_lexer_state_t *state, size_t start) {
    while (isident2(peek_char(state))) {
        next_char(state);
    }

    size_t end = state->offset - 1;

    char *ident = state_substring(state, start, end);

    c11_keyword_t keyword = (c11_keyword_t)map_get(c11_keywords, ident);
    
    if (keyword != KEY_INVALID) {
        c11_token_t token = finish_token(state, TOK_KEYWORD);
        token.keyword = keyword;
        return token;
    }

    c11_token_t token = finish_token(state, TOK_IDENT);
    token.ident = ident;
    return token;
}

static bool eat_char(c11_lexer_state_t *state, char c) {
    if (peek_char(state) == c) {
        next_char(state);
        return true;
    }

    return false;
}

static c11_token_t finish_string(c11_lexer_state_t *state, size_t offset) {
    while (true) {
        char c = peek_char(state);
        if (c == '"') {
            next_char(state);
            break;
        }

        if (c == '\0') {
            return finish_error(state, "unterminated string");
        }

        if (c == '\n') {
            return finish_error(state, "newline in string");
        }

        eat_char(state, '\\');
        next_char(state);
    }

    char *text = state_substring(state, offset, state->offset - 1);

    c11_token_t tok = finish_token(state, TOK_STRING);
    tok.string = text;
    return tok;
}

static c11_token_t next_token(c11_lexer_state_t *state) {
    char c = begin_token(state);
    size_t offset = state->offset - 1;

    if (c == '\0') {
        return finish_token(state, TOK_EOF);
    } else if (isident1(c)) {
        return finish_ident(state, offset);
    } else if (c == '"') {
        return finish_string(state, offset);
    }

    switch (c) {
    case '(': return finish_keyword(state, KEY_LPAREN);
    case ')': return finish_keyword(state, KEY_RPAREN);
    case '{': return finish_keyword(state, KEY_LBRACE);
    case '}': return finish_keyword(state, KEY_RBRACE);
    case '*': return finish_keyword(state, KEY_STAR);
    case ';': return finish_keyword(state, KEY_SEMICOLON);
    case ',': return finish_keyword(state, KEY_COMMA);
    case '.':
        if (eat_char(state, '.')) {
            if (eat_char(state, '.')) {
                return finish_keyword(state, KEY_DOT3);
            } else {
                return finish_keyword(state, KEY_DOT2);
            }
        } else {
            return finish_keyword(state, KEY_DOT);
        }
    default: break;
    }

    return finish_error(state, "unexpected character");
}

c11_token_t c11_lexer_next(c11_lexer_t *lexer) {
    c11_token_t token = lexer->token;
    lexer->token = next_token(lexer->state);
    return token;
}

c11_token_t c11_lexer_peek(c11_lexer_t *lexer) {
    return lexer->token;
}

bool tok_is_key(c11_token_t tok, c11_keyword_t keyword) {
    return tok.type == TOK_KEYWORD && tok.keyword == keyword;
}

bool tok_is_ident(c11_token_t tok) {
    return tok.type == TOK_IDENT;
}
