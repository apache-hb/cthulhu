#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report.h"

typedef struct {
    reports_t *reports;
    stream_t *stream;
    size_t depth;
} wasm_t;

static void wasm_indent(wasm_t *wasm) {
    wasm->depth += 1;
}

static void wasm_dedent(wasm_t *wasm) {
    wasm->depth -= 1;
}

static void wasm_begin(wasm_t *wasm, const char *str) {
    for (size_t i = 0; i < wasm->depth; i++) {
        stream_write(wasm->stream, "  ");
    }

    stream_write(wasm->stream, "(");
    stream_write(wasm->stream, str);
    stream_write(wasm->stream, "\n");

    wasm_indent(wasm);
}

static void wasm_end(wasm_t *wasm) {
    stream_write(wasm->stream, ")\n");

    wasm_dedent(wasm);
}

static void wasm_write(wasm_t *wasm, const char *str) {
    for (size_t i = 0; i < wasm->depth; i++) {
        stream_write(wasm->stream, "  ");
    }

    stream_write(wasm->stream, str);
    stream_write(wasm->stream, "\n");
}

static const char *get_width(wasm_t *wasm, digit_t digit) {
    switch (digit) {
    case DIGIT_CHAR:
    case DIGIT_SHORT:
    case DIGIT_INT: 
        return "32";

    case DIGIT_LONG:
    case DIGIT_SIZE: // TODO: wrong wrong wrong wrong
    case DIGIT_PTR:  //       awfully wrong, wasm has memory segments
                     //       and we need to respect that in our type system
        return "64";

    default:
        ctu_assert(wasm->reports, "unhandled digit %d", digit);
        return "error";
    }
}

static char *emit_digit(wasm_t *wasm, const hlir_t *node) {
    const char *sign = node->sign == SIGN_UNSIGNED ? "u" : "i";
    const char *width = get_width(wasm, node->width);

    return format("%s%s", sign, width);
}

static char *emit_type(wasm_t *wasm, const hlir_t *node) {
    switch (node->type) {
    case HLIR_DIGIT: return emit_digit(wasm, node);
    default: 
        ctu_assert(wasm->reports, "emit-type unknown type %d", node->type);
        return ctu_strdup("error");
    }
}

static void emit_node(wasm_t *wasm, const hlir_t *node);

static void emit_module_node(wasm_t *wasm, const hlir_t *node) {
    wasm_begin(wasm, "module");

    for (size_t i = 0; i < vector_len(node->globals); i++) {
        emit_node(wasm, vector_get(node->globals, i));
    }

    for (size_t i = 0; i < vector_len(node->functions); i++) {
        emit_node(wasm, vector_get(node->functions, i));
    }

    wasm_end(wasm);
}

static void emit_function_node(wasm_t *wasm, const hlir_t *node) {
    if (node->body == NULL) {
        return;
    }

    wasm_begin(wasm, format("func $%s", nameof_hlir(node)));

    const hlir_t *signature = typeof_hlir(node);

    for (size_t i = 0; i < vector_len(signature->params); i++) {
        const hlir_t *param = vector_get(signature->params, i);
        char *type = emit_type(wasm, param);
        wasm_write(wasm, format("(param %s)", type));
    }

    for (size_t i = 0; i < vector_len(node->locals); i++) {
        const hlir_t *local = vector_get(node->locals, i);
        char *type = emit_type(wasm, typeof_hlir(local));
        wasm_write(wasm, format("(local %s)", type));
    }

    if (!hlir_is(signature->result, HLIR_VOID)) {
        char *result = emit_type(wasm, signature->result);
        wasm_write(wasm, format("(result %s)", result));
    }


    emit_node(wasm, node->body);

    wasm_end(wasm);
}

static void emit_stmts_node(wasm_t *wasm, const hlir_t *node) {
    for (size_t i = 0; i < vector_len(node->stmts); i++) {
        emit_node(wasm, vector_get(node->stmts, i));
    }
}

static void emit_assign_node(wasm_t *wasm, const hlir_t *node) {
    emit_node(wasm, node->src);

    const hlir_t *type = typeof_hlir(node->dst);
    char *str = emit_type(wasm, type);
    wasm_write(wasm, format("%s.store %s", str, nameof_hlir(node->dst)));
}

static void emit_digit_literal_node(wasm_t *wasm, const hlir_t *node) {
    char *str = emit_type(wasm, typeof_hlir(node));
    wasm_write(wasm, format("%s.const %s", str, mpz_get_str(NULL, 10, node->digit)));
}

static void emit_value_node(wasm_t *wasm, const hlir_t *node) {
    wasm_begin(wasm, "global");
}

static void emit_node(wasm_t *wasm, const hlir_t *node) {
    switch (node->type) {
    case HLIR_MODULE:
        emit_module_node(wasm, node);
        break;

    case HLIR_FUNCTION:
        emit_function_node(wasm, node);
        break;

    case HLIR_STMTS:
        emit_stmts_node(wasm, node);
        break;

    case HLIR_ASSIGN:
        emit_assign_node(wasm, node);
        break;

    case HLIR_DIGIT_LITERAL:
        emit_digit_literal_node(wasm, node);
        break;

    case HLIR_GLOBAL:
        emit_value_node(wasm, node);
        break;

    default:
        ctu_assert(wasm->reports, "unhandled node type %d", node->type);
        break;
    }
}

void wasm_emit_tree(reports_t *reports, const hlir_t *hlir) {
    wasm_t wasm = { reports, stream_new(0x1000), 0 };
    emit_node(&wasm, hlir);

    printf("%s", stream_data(wasm.stream));
}
