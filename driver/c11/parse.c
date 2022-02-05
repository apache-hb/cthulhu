#include "parse.h"

typedef enum {
    TAG_STRUCTS,
    TAG_ENUMS,
    TAG_FUNCS,
    TAG_TYPEDEFS,
    TAG_VARS,
    
    TAG_MAX
} c11_tag_t;

hlir_t *c11_compile(reports_t *reports, scan_t *scan) {
    c11_lexer_t *lex = c11_lexer_new(scan);

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
