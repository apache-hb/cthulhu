#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/util/report.h"

typedef struct {
    reports_t *reports;
    stream_t *stream;
    size_t depth;
} wasm_t;

static void wasm_begin(wasm_t *wasm, const char *str) {
    for (size_t i = 0; i < wasm->depth; i++) {
        stream_write(wasm->stream, "  ");
    }

    stream_write(wasm->stream, "(");
    stream_write(wasm->stream, str);
    stream_write(wasm->stream, "\n");

    wasm->depth += 1;
}

static void wasm_end(wasm_t *wasm) {
    stream_write(wasm->stream, ")\n");

    wasm->depth -= 1;
}

static void emit_node(wasm_t *wasm, const hlir_t *node);

static void emit_module_node(wasm_t *wasm, const hlir_t *node) {
    wasm_begin(wasm, "module");

    wasm_end(wasm);
}

static void emit_node(wasm_t *wasm, const hlir_t *node) {
    switch (node->type) {
    case HLIR_MODULE:
        emit_module_node(wasm, node);
        break;

    default:
        ctu_assert(wasm->reports, "unhandled node type %d", node->type);
        break;
    }
}

void wasm_emit_tree(reports_t *reports, const hlir_t *hlir) {
    wasm_t wasm = { reports, stream_new(0x1000) };
}
