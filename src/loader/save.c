#include "common.h"

typedef struct {
    reports_t *reports; // report sink
    
    map_t *data; // map of node -> id

    vector_t *strings; // string table
    vector_t *spans; // span table
    vector_t *arrays; // array table
    offset_t nspans;

    offset_t offsets[HLIR_TOTAL]; // total number of each node type
    vector_t *nodes[HLIR_TOTAL]; // all nodes of each type
} writer_t;

#define WRITE_NODE(type, name, ...) do { name data = __VA_ARGS__; vector_write_bytes(&writer->nodes[type], &data, sizeof(data)); } while (0)

static node_index_t EMPTY_NODE = {
    .type = UINT8_MAX,
    .index = 0
};

static node_index_t save_node(writer_t *writer, const hlir_t *node);

static node_index_t save_opt_node(writer_t *writer, const hlir_t *node) {
    node_index_t index = EMPTY_NODE;
    
    if (node) {
        index = save_node(writer, node);
    }

    return index;
}

static offset_t save_node_array(writer_t *writer, vector_t *vector) {
    size_t len = vector_len(vector);
    size_t size = sizeof(node_array_t) + (sizeof(node_index_t) * len);
    node_array_t *array = ctu_malloc(size);
    array->size = len;

    for (size_t i = 0; i < len; i++) {
        array->offsets[i] = save_node(writer, vector_get(vector, i));
    }

    vector_write_bytes(&writer->arrays, array, size);

    ctu_free(array);

    return vector_len(writer->arrays);
}

static offset_t save_string(writer_t *writer, const char *str) {
    if (str == NULL) {
        return UINT32_MAX;
    }

    vector_write(&writer->strings, str, true);
    return vector_len(writer->strings);
}

static offset_t save_span(writer_t *writer, const node_t *node) {
    if (node == NULL) {
        return UINT32_MAX;
    }
    vector_write_bytes(&writer->spans, &node->where, sizeof(node->where));
    return writer->nspans++;
}

static node_header_t build_header(writer_t *writer, const node_t *node, node_index_t type) {
    node_header_t header = {
        .span = save_span(writer, node),
        .type = type
    };

    return header;
}

static node_header_t new_header(writer_t *writer, const hlir_t *node) {
    const hlir_t *kind = typeof_hlir(node);
    node_index_t type = EMPTY_NODE;
    
    if (!hlir_is_sentinel(kind)) {
        type = save_node(writer, kind);
    }

    return build_header(writer, node->node, type);
}

static void save_bool_literal(writer_t *writer, const hlir_t *node) {
    WRITE_NODE(HLIR_BOOL_LITERAL, bool_node_t, {
        .header = new_header(writer, node),
        .value = node->boolean
    });
}

static void save_digit_literal(writer_t *writer, const hlir_t *node) {
    char *digit = mpz_get_str(NULL, DIGIT_BASE, node->digit);
    offset_t offset = save_string(writer, digit);

    WRITE_NODE(HLIR_DIGIT_LITERAL, digit_node_t, {
        .header = new_header(writer, node),
        .digit = offset
    });
}

static void save_string_literal(writer_t *writer, const hlir_t *node) {
    offset_t offset = save_string(writer, node->string);

    WRITE_NODE(HLIR_STRING_LITERAL, string_node_t, {
        .header = new_header(writer, node),
        .string = offset
    });
}

static void save_module_node(writer_t *writer, const hlir_t *node) {
    offset_t globals = save_node_array(writer, node->globals);
    WRITE_NODE(HLIR_MODULE, module_node_t, {
        .header = new_header(writer, node),
        .globals = globals
    });
}

static void save_value_node(writer_t *writer, const hlir_t *node) {
    offset_t name = save_string(writer, node->name);
    node_index_t init = save_opt_node(writer, node->value);
    
    WRITE_NODE(HLIR_VALUE, value_node_t, {
        .header = new_header(writer, node),
        .name = name,
        .init = init
    });
}

static void save_digit_type(writer_t *writer, const hlir_t *node) {
    WRITE_NODE(HLIR_DIGIT, digit_type_node_t, {
        .header = new_header(writer, node),
        .name = save_string(writer, node->name),
        .width = node->width,
        .sign = node->sign
    });
}

static void save_type_type(writer_t *writer, const hlir_t *node) {
    WRITE_NODE(HLIR_TYPE, metatype_node_t, {
        .header = build_header(writer, node->node, EMPTY_NODE),
        .name = save_string(writer, node->name)
    });
}

static node_index_t save_node(writer_t *writer, const hlir_t *node) {
    hlir_type_t type = node->type;
    uintptr_t id = (uintptr_t)map_get_ptr_default(writer->data, node, (void*)SIZE_MAX);

    node_index_t index = {
        .type = type,
        .index = id
    };

    if (id != SIZE_MAX) { return index; }

    index.index = writer->offsets[type]++;
    map_set_ptr(writer->data, node, (void*)id);

    switch (type) {
    case HLIR_BOOL_LITERAL: 
        save_bool_literal(writer, node);
        break;
    case HLIR_DIGIT_LITERAL:
        save_digit_literal(writer, node);
        break;
    case HLIR_STRING_LITERAL:
        save_string_literal(writer, node);
        break;

    case HLIR_DIGIT:
        save_digit_type(writer, node);
        break;
    case HLIR_TYPE:
        save_type_type(writer, node);
        break;

    case HLIR_VALUE:
        save_value_node(writer, node);
        break;

    case HLIR_MODULE:
        save_module_node(writer, node);
        break;

    case HLIR_ERROR: {
        message_t *id = report(writer->reports, INTERNAL, node->node, "error node should never appear in output");
        report_note(id, "error: %s", node->error);
        break;
    }
    
    default:
        ctu_assert(writer->reports, "unhandled node type: %d", type);
    }

    return index;
}

void save_module(reports_t *reports, hlir_t *module, const char *path) {
    UNUSED(module);

    file_t file = ctu_fopen(path, "wb");
    if (!file_valid(&file)) { 
        report(reports, ERROR, NULL, "could not open file: %s (errno %d)", path, errno);
        return; 
    }

    writer_t writer = {
        .reports = reports,
        .data = map_new(MAP_MASSIVE),
        .strings = vector_new(0x1000),
        .spans = vector_new(0x1000 * sizeof(where_t)),
        .arrays = vector_new(0x1000),
        .nspans = 0
    };
    
    for (int i = 0; i < HLIR_TOTAL; i++) {
        writer.offsets[i] = 0;
        writer.nodes[i] = vector_new(0x1000);
    }

    save_node(&writer, module);

    header_t header = {
        .magic = HEADER_MAGIC,
        .version = CURRENT_VERSION,
        .strings = sizeof(header_t),
        .spans = sizeof(header_t) + vector_len(writer.strings)
    };

    size_t offset = 0;
    for (int i = 0; i < HLIR_TOTAL; i++) {
        header.counts[i] = offset + sizeof(header_t);
        offset += vector_len(writer.nodes[i]);
    }

    fwrite(&header, sizeof(header), 1, file.file);

    fwrite(vector_data(writer.strings), vector_len(writer.strings), 1, file.file);
    fwrite(vector_data(writer.spans), vector_len(writer.spans), 1, file.file);

    for (int i = 0; i < HLIR_TOTAL; i++) {
        fwrite(vector_data(writer.nodes[i]), vector_len(writer.nodes[i]), 1, file.file);
    }

    ctu_close(&file);
}
