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
    scan_t scan;
} memory_t;

static bool check_range(memory_t *memory, size_t offset, size_t length) {
    if (offset + length > memory->length) {
        message_t *id = report(memory->reports, ERROR, NULL, "corrupted ir file");
        report_note(id, "invalid read of %zu bytes at offset %zu requested by file", length, offset);
        return false;
    }

    return true;
}

static void read_bytes(memory_t *memory, void *dst, size_t offset, size_t length) {
    if (!check_range(memory, offset, length)) {
        memset(dst, 0, length);
        return;
    }

    memcpy(dst, memory->data + offset, length);
}

static node_t *read_span(memory_t *memory, size_t offset) {
    where_t where;
    read_bytes(memory, &where, offset + memory->header.spans, sizeof(where_t));

    logverbose("span@%zu = %zu:%zu", offset, where.first_line, where.first_column);

    return node_new(&memory->scan, where);
}

static char *read_string(memory_t *memory, size_t offset) {
    if (!check_range(memory, offset, 0)) {
        return ctu_strdup("");
    }

    char *str = ctu_strdup(memory->data + memory->header.strings + offset);

    logverbose("string@%zu = `%s`", offset, str);

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

static vector_t *read_node_array(memory_t *memory, size_t offset) {
    offset += memory->header.arrays;
    
    length_t length;
    read_bytes(memory, &length, offset, sizeof(length_t));

    vector_t *vector = vector_of(length);

    logverbose("array@%zu = %zu", offset, length);

    for (length_t i = 0; i < length; i++) {
        node_index_t index;
        read_bytes(memory, &index, offset + sizeof(length_t) + (i * sizeof(index)), sizeof(index));
        uint8_t type = index.type;
        size_t offset = get_offset(memory, type) + (get_length(memory, type) * index.index);

        hlir_t *node = read_node(memory, index.type, offset);
        vector_set(vector, i, node);
    }

    return vector;
}

static hlir_t *read_module(memory_t *memory, size_t offset) {
    module_node_t node;
    read_bytes(memory, &node, offset, sizeof(module_node_t));

    node_t *span = read_span(memory, node.header.span);
    char *name = read_string(memory, node.name);

    vector_t *globals = read_node_array(memory, node.globals);
    vector_t *types = read_node_array(memory, node.types);

    hlir_t *hlir = hlir_new_module(span, name);
    hlir_build_module(hlir, globals, vector_of(0), types);

    return hlir;
}

static hlir_t *read_node(memory_t *memory, uint8_t type, size_t offset) {
    switch (type) {
    case HLIR_MODULE:
        return read_module(memory, offset);

    default:
        ctu_assert(memory->reports, "read-node called with invalid type %d", type);
        return hlir_error(NULL, "invalid read-node");
    }
}

hlir_t *load_module(reports_t *reports, const char *path) {
    file_t file = ctu_fopen(path, "rb");
    if (!file_valid(&file)) { return NULL; }

    char *data = ctu_mmap(&file);

    memory_t memory = {
        .reports = reports,
        .data = data,
        .length = file_size(file.file),

        .scan = scan_string(reports, NULL, path, data)
    };

    read_bytes(&memory, &memory.header, 0, sizeof(memory.header));
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

    logverbose("loaded ir file: %s", path);
    logverbose(" - magic: %x", header.magic);
    logverbose(" - version: %d.%d.%d", VERSION_MAJOR(header.version), VERSION_MINOR(header.version), VERSION_PATCH(header.version));
    logverbose(" - strings: %d", header.strings);
    logverbose(" - spans: %d", header.spans);

    logverbose("counters:");
    for (int i = 0; i < TOTAL_COUNTS; i++) {
        logverbose(" node[%d] offset=%d length=%d", i, header.offsets[i] - sizeof(header_t), header.counts[i]);
    }

    for (int i = 0; i < TOTAL_COUNTS; i++) {
        memory.nodes[i] = malloc(sizeof(hlir_t*) * header.counts[i]);
    }

    for (int i = 0; i < TOTAL_COUNTS; i++) {
        size_t size = get_node_size(reports, i);

        for (length_t j = 0; j < header.counts[i]; j++) {
            size_t offset = header.offsets[i] + (j * size);
            logverbose("reading node[%d] at %d", i, offset);
            memory.nodes[i][j] = read_node(&memory, i, offset);
        }
    }

    return NULL;
}
