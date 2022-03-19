#include "common.h"

#include "cthulhu/ast/compile.h"

#include <string.h>

typedef struct {
    // file data
    reports_t *reports;
    const char *data;
    size_t length;

    // parse data
    header_t header;
    hlir_t **nodes[TOTAL_COUNTS];
    scan_t *scan;
} memory_t;

static bool check_range(memory_t *memory, size_t offset, size_t length, const char *reason) {
    if (offset + length > memory->length) {
        message_t *id = report(memory->reports, ERROR, NULL, "corrupted ir file");
        report_note(id, "invalid read of %zu bytes at offset %zu requested by %s", length, offset, reason);
        return false;
    }

    return true;
}

static void read_bytes(memory_t *memory, void *dst, size_t offset, size_t length, const char *reason) {
    if (!check_range(memory, offset, length, reason)) {
        memset(dst, 0, length);
        return;
    }

    memcpy(dst, memory->data + offset, length);
}

static const node_t *read_span(memory_t *memory, size_t offset, const char *reason) {
    if (offset == UINT32_MAX) {
        return node_builtin();
    }

    offset += memory->header.spans;

    where_t where;
    read_bytes(memory, &where, offset, sizeof(where_t), reason);

    return node_new(memory->scan, where);
}

static char *read_string(memory_t *memory, size_t offset, const char *reason) {
    size_t where = memory->header.strings + offset;

    if (!check_range(memory, where, 0, reason)) {
        return ctu_strdup("");
    }

    char *str = ctu_strdup(memory->data + where);

    return str;
}

static offset_t get_offset(memory_t *memory, uint8_t value) {
    if (value > TOTAL_COUNTS) {
        message_t *id = report(memory->reports, ERROR, NULL, "corrupted ir file");
        report_note(id, "invalid node type %d", value);
        return 0;
    }

    return memory->header.offsets[value];
}

static length_t get_length(memory_t *memory, uint8_t value) {
    if (value > TOTAL_COUNTS) {
        message_t *id = report(memory->reports, ERROR, NULL, "corrupted ir file");
        report_note(id, "invalid node type %d", value);
        return 0;
    }

    return memory->header.counts[value];
}

static hlir_t *read_node(memory_t *memory, uint8_t type, size_t offset);

static hlir_t *get_node(memory_t *memory, node_index_t index) {
    uint8_t type = index.type;
    size_t offset = get_offset(memory, type) + (get_length(memory, type) * index.index);

    return read_node(memory, index.type, offset);
}

static hlir_t *get_opt_node(memory_t *memory, node_index_t index) {
    if (index.type == UINT8_MAX) { return NULL; }
    if (index.index == UINT32_MAX) { return NULL; }

    return get_node(memory, index);
}

static vector_t *read_node_array(memory_t *memory, size_t offset) {
    offset += memory->header.arrays;

    length_t length;
    read_bytes(memory, &length, offset, sizeof(length_t), "array length header");

    vector_t *vector = vector_of(length);

    for (length_t i = 0; i < length; i++) {
        node_index_t index;
        read_bytes(memory, &index, offset + sizeof(length_t) + (i * sizeof(index)), sizeof(index), "array element");

        hlir_t *node = get_node(memory, index);
        vector_set(vector, i, node);
    }

    return vector;
}

static hlir_t *read_module(memory_t *memory, size_t offset) {
    module_node_t node;
    read_bytes(memory, &node, offset, sizeof(module_node_t), "module node");

    const node_t *span = read_span(memory, node.header.span, "module span");
    char *name = read_string(memory, node.name, "module name");

    vector_t *globals = read_node_array(memory, node.globals);
    vector_t *types = read_node_array(memory, node.types);

    hlir_t *hlir = hlir_new_module(span, name);
    hlir_build_module(hlir, globals, vector_of(0), types);

    return hlir;
}

static hlir_t *read_value(memory_t *memory, size_t offset) {
    value_node_t node;
    read_bytes(memory, &node, offset, sizeof(value_node_t), "value node");

    char *name = read_string(memory, node.name, "value name read");
    const node_t *span = read_span(memory, node.header.span, name);
    hlir_t *init = get_opt_node(memory, node.init);

    hlir_t *hlir = hlir_new_value(span, name, NULL);
    hlir_build_value(hlir, init);

    return hlir;
}

static hlir_t *read_digit_literal(memory_t *memory, size_t offset) {
    digit_node_t node;
    read_bytes(memory, &node, offset, sizeof(digit_node_t), "digit node");

    const node_t *span = read_span(memory, node.header.span, "digit span");
    char *name = read_string(memory, node.digit, "digit literal read");

    mpz_t mpz;
    mpz_init_set_str(mpz, name, DIGIT_BASE);

    return hlir_digit_literal(span, NULL, mpz);
}

static hlir_t *read_string_literal(memory_t *memory, size_t offset) {
    string_node_t node;
    read_bytes(memory, &node, offset, sizeof(string_node_t), "string node");

    const node_t *span = read_span(memory, node.header.span, "string span");
    char *name = read_string(memory, node.string, "string literal read");

    return hlir_string_literal(span, NULL, name);
}

static hlir_t *read_bool_literal(memory_t *memory, size_t offset) {
    bool_node_t node;
    read_bytes(memory, &node, offset, sizeof(bool_node_t), "bool node");

    const node_t *span = read_span(memory, node.header.span, "bool span");

    return hlir_bool_literal(span, NULL, node.value);
}

static hlir_t *read_node(memory_t *memory, uint8_t type, size_t offset) {
    switch (type) {
    case HLIR_DIGIT_LITERAL:
        return read_digit_literal(memory, offset);

    case HLIR_BOOL_LITERAL:
        return read_bool_literal(memory, offset);

    case HLIR_STRING_LITERAL:
        return read_string_literal(memory, offset);

    case HLIR_MODULE:
        return read_module(memory, offset);

    case HLIR_VALUE:
        return read_value(memory, offset);

    default:
        ctu_assert(memory->reports, "read-node called with invalid type %d", type);
        return hlir_error(NULL, "invalid read-node");
    }
}

hlir_t *load_module(reports_t *reports, const char *path) {
    file_t file = ctu_fopen(path, "rb");
    if (!file_valid(&file)) { return NULL; }

    char *data = ctu_mmap(&file);

    scan_t scan = scan_string(reports, NULL, path, data);

    memory_t memory = {
        .reports = reports,
        .data = data,
        .length = file_size(file.file),

        .scan = BOX(scan)
    };

    read_bytes(&memory, &memory.header, 0, sizeof(memory.header), "file header");
    header_t header = memory.header;

    if (header.magic != HEADER_MAGIC) {
        message_t *id = report(reports, ERROR, NULL, "corrupted or invalid ir file: %s", path);
        report_note(id, "file magic was `%x`, expected `%x`", header.magic, HEADER_MAGIC);
        return NULL;
    }

    if (VERSION_MAJOR(header.version) > VERSION_MAJOR(CURRENT_VERSION)) {
        message_t *id = report(reports, ERROR, NULL, "unsupported ir version: %s", path);
        report_note(id, "file version was `%d.%d`, loader only supports up to `%d.%d`", 
            VERSION_MAJOR(header.version), 
            VERSION_MINOR(header.version),
            VERSION_MAJOR(CURRENT_VERSION),
            VERSION_MINOR(CURRENT_VERSION)
        );
        return NULL;
    }

    char *language = read_string(&memory, header.language, "module language");
    memory.scan->language = language;

    for (int i = 0; i < TOTAL_COUNTS; i++) {
        memory.nodes[i] = malloc(sizeof(hlir_t*) * header.counts[i]);
    }

    for (int i = 0; i < TOTAL_COUNTS; i++) {
        size_t size = get_node_size(reports, i);

        for (length_t j = 0; j < header.counts[i]; j++) {
            size_t offset = header.offsets[i] + (j * size);
            memory.nodes[i][j] = read_node(&memory, j, offset);
        }
    }

    if (header.counts[HLIR_MODULE] > 1) {
        report(reports, WARNING, NULL, "multiple modules in file: %s", path);
    }

    return memory.nodes[HLIR_MODULE][0];
}
