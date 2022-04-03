#include "cthulhu/debug/hlir.h"

typedef struct {
    reports_t *reports;
    int depth;
} write_t;

static void write(write_t *out, const char *tag, const char *msg) {
    for (int i = 0; i < out->depth; i++) {
        printf(" ");
    }
    
    if (tag != NULL) {
        printf("%s: ", tag);
    }

    printf("%s\n", msg);
}

static void into(write_t *out) {
    out->depth += 2;
}

static void outof(write_t *out) {
    out->depth -= 2;
}

static void debug_node(write_t *out, const char *tag, bool detail, const hlir_t *hlir);

static void debug_vec(write_t *out, vector_t *vec, bool detail, const char *msg) {
    size_t len = vector_len(vec);

    write(out, NULL, format("- %s[%zu]", msg, len));
    if (len == 0) { return; }

    into(out);

    for (size_t i = 0; i < len; i++) {
        debug_node(out, format("[%zu]", i), detail, vector_get(vec, i));
    }

    outof(out);
}

static void debug_node(write_t *out, const char *tag, bool detail, const hlir_t *hlir) {
    if (hlir == NULL) { return; }

    switch (hlir->type) {
    /* expressions */
    case HLIR_DIGIT_LITERAL:
        write(out, tag, mpz_get_str(NULL, 10, hlir->digit));
        break;

    case HLIR_STRING_LITERAL:
        write(out, tag, hlir->string);
        break;

    case HLIR_BOOL_LITERAL:
        write(out, tag, hlir->boolean ? "true" : "false");
        break;

    case HLIR_NAME:
        write(out, tag, "name");
        if (detail) {
            into(out);
                debug_node(out, "src", false, hlir->read);
            outof(out);
        }
        break;

    /* statements */

    case HLIR_STMTS:
        debug_vec(out, hlir->stmts, false, "stmts");
        break;

    case HLIR_ASSIGN:
        write(out, tag, "assign");
        into(out);
            debug_node(out, "dst", false, hlir->dst);
            debug_node(out, "src", false, hlir->src);
        outof(out);
        break;

    case HLIR_LOOP:
        write(out, tag, "loop");
        into(out);
            debug_node(out, "cond", false, hlir->cond);
            debug_node(out, "body", false, hlir->then);
            debug_node(out, "else", false, hlir->other);
        outof(out);
        break;

    /* types */

    case HLIR_DIGIT:
        write(out, tag, format("digit `%s` (sign = %s, width = %s)", nameof_hlir(hlir), sign_name(hlir->sign), digit_name(hlir->width)));
        break;

    case HLIR_BOOL:
        write(out, tag, format("bool `%s`", nameof_hlir(hlir)));
        break;
    
    case HLIR_STRING:
        write(out, tag, format("string `%s`", nameof_hlir(hlir)));
        break;

    case HLIR_VOID:
        write(out, tag, format("void `%s`", nameof_hlir(hlir)));
        break;

    case HLIR_CLOSURE:
        write(out, tag, format("closure `%s`", nameof_hlir(hlir)));
        into(out);
            debug_vec(out, hlir->params, false, "params");
            debug_node(out, "result", false, hlir->result);
            write(out, "variadic", hlir->variadic ? "true" : "false");
        outof(out);
        break;

    /* decls */

    case HLIR_GLOBAL:
        write(out, tag, format("value `%s`", nameof_hlir(hlir)));
        if (detail) {
            into(out);
                debug_node(out, "type", true, typeof_hlir(hlir));
                debug_node(out, "value", true, hlir->value);
            outof(out);
        }
        break;

    case HLIR_FUNCTION:
        write(out, tag, format("function `%s`", nameof_hlir(hlir)));
        into(out);
            debug_node(out, "signature", true, typeof_hlir(hlir));
        outof(out);
        debug_vec(out, hlir->locals, true, "locals");
        into(out);
            debug_node(out, "body", true, hlir->body);
        outof(out);
        break;

    case HLIR_MODULE:
        write(out, tag, format("module `%s`", nameof_hlir(hlir)));
        debug_vec(out, hlir->types, true, "types");
        debug_vec(out, hlir->globals, true, "globals");
        debug_vec(out, hlir->functions, true, "functions");
        break;

    default:
        ctu_assert(out->reports, "unexpected node type %d", hlir->type);
        break;
    }
}

void debug_hlir(reports_t *reports, const hlir_t *hlir) {
    write_t out = { reports, 0 };

    debug_node(&out, NULL, true, hlir);
}
