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

static void debug_node(write_t *out, const char *tag, const hlir_t *hlir);

static void debug_vec(write_t *out, vector_t *vec, const char *msg) {
    size_t len = vector_len(vec);

    write(out, NULL, format(" - %s[%zu]", msg, len));
    into(out);

    for (size_t i = 0; i < len; i++) {
        debug_node(out, format("[%zu]", i), vector_get(vec, i));
    }

    outof(out);
}

static void debug_node(write_t *out, const char *tag, const hlir_t *hlir) {
    if (hlir == NULL) { return; }

    switch (hlir->type) {
    case HLIR_MODULE:
        write(out, tag, format("module `%s`", nameof_hlir(hlir)));
        debug_vec(out, hlir->types, "types");
        debug_vec(out, hlir->globals, "globals");
        break;

    case HLIR_VALUE:
        write(out, tag, format("value `%s`", nameof_hlir(hlir)));
        into(out);
            debug_node(out, "type", typeof_hlir(hlir));
            debug_node(out, "value", hlir->value);
        outof(out);
        break;

    case HLIR_DIGIT:
        write(out, tag, format("digit `%s` (sign = %s, width = %s)", nameof_hlir(hlir), sign_name(hlir->sign), digit_name(hlir->width)));
        break;

    case HLIR_DIGIT_LITERAL:
        write(out, tag, mpz_get_str(NULL, 10, hlir->digit));
        break;

    default:
        ctu_assert(out->reports, "unexpected node type %d", hlir->type);
    }
}

void debug_hlir(reports_t *reports, const hlir_t *hlir) {
    write_t out = { reports, 0 };

    debug_node(&out, NULL, hlir);
}
