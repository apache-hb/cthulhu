#include "cthulhu/loader/hlir.h"

#include "cthulhu/ast/compile.h"

///
/// layouts
///

typedef enum {
    SPAN_INDEX = HLIR_TOTAL,
    LAYOUTS_TOTAL
} hlir_layouts_t;

// header data
enum { HEADER_LANGUAGE, HEADER_PATH };
static const field_t HEADER_FIELDS[] = {
    [HEADER_LANGUAGE] = FIELD("language", FIELD_STRING),
    [HEADER_PATH] = FIELD("path", FIELD_STRING)
};
static const layout_t HEADER_LAYOUT = LAYOUT("header", HEADER_FIELDS);

// span data
enum { SPAN_FIRST_LINE, SPAN_FIRST_COLUMN, SPAN_LAST_LINE, SPAN_LAST_COLUMN };
static const field_t SPAN_FIELDS[] = {
    [SPAN_FIRST_LINE] = FIELD("first-line", FIELD_INT),
    [SPAN_FIRST_COLUMN] = FIELD("first-column", FIELD_INT),
    [SPAN_LAST_LINE] = FIELD("last-line", FIELD_INT),
    [SPAN_LAST_COLUMN] = FIELD("last-column", FIELD_INT)
};
static const layout_t SPAN_LAYOUT = LAYOUT("span", SPAN_FIELDS);

enum { MODULE_SPAN, MODULE_NAME, MODULE_GLOBALS };
static const field_t MODULE_FIELDS[] = {
    [MODULE_SPAN] = FIELD("span", FIELD_REFERENCE),
    [MODULE_NAME] = FIELD("name", FIELD_STRING),
    [MODULE_GLOBALS] = FIELD("globals", FIELD_ARRAY)
};
static const layout_t MODULE_LAYOUT = LAYOUT("module", MODULE_FIELDS);

#if 0
enum { DIGIT_LITERAL_SPAN, DIGIT_LITERAL_TYPE, DIGIT_LITERAL_VALUE };
static const field_t DIGIT_LITERAL_FIELDS[] = {
    [DIGIT_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [DIGIT_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [DIGIT_LITERAL_VALUE] = FIELD("value", FIELD_INT)
};
static const layout_t DIGIT_LITERAL_LAYOUT = LAYOUT("digit-literal", DIGIT_LITERAL_FIELDS);

enum { STRING_LITERAL_SPAN, STRING_LITERAL_TYPE, STRING_LITERAL_VALUE };
static const field_t STRING_FIELDS[] = {
    [STRING_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [STRING_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [STRING_LITERAL_VALUE] = FIELD("value", FIELD_STRING)
};
static const layout_t STRING_LITERAL_LAYOUT = LAYOUT("string-literal", STRING_FIELDS);
#endif

static const layout_t TYPES[LAYOUTS_TOTAL] = {
    [SPAN_INDEX] = SPAN_LAYOUT,
    [HLIR_MODULE] = MODULE_LAYOUT,

#if 0
    [HLIR_DIGIT_LITERAL] = DIGIT_LITERAL_LAYOUT,
    [HLIR_STRING_LITERAL] = STRING_LITERAL_LAYOUT
#endif
};

static const format_t GLOBAL = FORMAT(HEADER_LAYOUT, TYPES);

#define HLIR_SUBMAGIC 0x484C4952 // 'HLIR'
#define HLIR_VERSION NEW_VERSION(0, 0, 1)

///
/// hlir loading 
///

typedef struct {
    scan_t *scan;
    data_t *data;
} load_t;

static node_t *load_span(load_t *load, index_t index) {
    value_t values[FIELDLEN(SPAN_FIELDS)];
    read_entry(load->data, index, values);
    
    where_t where = {
        .first_line = get_int(values[SPAN_FIRST_LINE]),
        .first_column = get_int(values[SPAN_FIRST_COLUMN]),
        .last_line = get_int(values[SPAN_LAST_LINE]),
        .last_column = get_int(values[SPAN_LAST_COLUMN])
    };

    return node_new(load->scan, where);
}

static hlir_t *load_node(load_t *load, index_t index);

static vector_t *load_array(load_t *load, array_t array) {
    size_t len = array.length;
    index_t *indices = ctu_malloc(sizeof(index_t) * len);
    vector_t *vec = vector_of(len);

    read_array(load->data, array, indices);

    for (size_t i = 0; i < len; i++) {
        hlir_t *hlir = load_node(load, indices[i]);
        vector_set(vec, i, hlir);
    }

    return vec;
}

static hlir_t *load_module_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(MODULE_FIELDS)];
    read_entry(load->data, index, values);

    const node_t *node = load_span(load, get_reference(values[MODULE_SPAN]));
    const char *name = get_string(values[MODULE_NAME]);
    vector_t *globals = load_array(load, get_array(values[MODULE_GLOBALS]));

    hlir_t *hlir = hlir_new_module(node, name);
    hlir_build_module(hlir, globals, vector_of(0), vector_of(0));

    return hlir;
}

static hlir_t *load_node(load_t *load, index_t index) {
    switch (index.type) {
    case HLIR_MODULE:
        return load_module_node(load, index);
    
    default:
        return NULL;
    }
}

hlir_t *load_module(reports_t *reports, const char *path) {
    value_t values[] = {
        [HEADER_LANGUAGE] = string_value("hlir"),
        [HEADER_PATH] = string_value(path)
    };

    record_t record = {
        .layout = &HEADER_LAYOUT,
        .values = values
    };

    header_t header = { 
        .reports = reports,
        .format = &GLOBAL,
        .path = path,
        .header = record,
        .submagic = HLIR_SUBMAGIC,
        .semver = HLIR_VERSION
    };

    data_t data;
    begin_load(&data, header);
    
    const char *language = get_string(data.header.header.values[HEADER_LANGUAGE]);
    const char *source = get_string(data.header.header.values[HEADER_PATH]);

    logverbose("loading(lang=%s, path=%s)", language, source);

    file_t file = ctu_fopen(source, "r");
    file_t *fp = BOX(file);

    scan_t scan = scan_file(reports, language, fp);

    load_t load = {
        .scan = BOX(scan),
        .data = &data
    };
    index_t index = { .type = HLIR_MODULE };
    hlir_t *hlir = load_node(&load, index);

    end_load(&data);

    return hlir;
}

///
/// hlir saving
///


static index_t save_span(data_t *data, const node_t *node) {
    value_t values[FIELDLEN(SPAN_FIELDS)] = {
        [SPAN_FIRST_LINE] = int_value(node->where.first_line),
        [SPAN_FIRST_COLUMN] = int_value(node->where.first_column),
        [SPAN_LAST_LINE] = int_value(node->where.last_line),
        [SPAN_LAST_COLUMN] = int_value(node->where.last_column)
    };
    
    return write_entry(data, SPAN_INDEX, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir);

static array_t save_array(data_t *data, vector_t *vec) {
    size_t len = vector_len(vec);
    index_t *indices = ctu_malloc(sizeof(index_t) * len);

    for (size_t i = 0; i < len; i++) {
        indices[i] = save_node(data, vector_get(vec, i));
    }

    return write_array(data, indices, len);
}

static index_t save_module_node(data_t *data, const hlir_t *hlir) {
    index_t span = save_span(data, hlir->node);
    array_t globals = save_array(data, hlir->globals);
    
    value_t values[] = {
        [MODULE_SPAN] = reference_value(span),
        [MODULE_NAME] = string_value(hlir->name),
        [MODULE_GLOBALS] = array_value(globals)
    };
    
    return write_entry(data, HLIR_MODULE, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_MODULE:
        return save_module_node(data, hlir);
    
    default:
        return (index_t){ 0 };
    }
}

void save_module(reports_t *reports, hlir_t *module, const char *path) {
    value_t values[] = {
        { .string = "hlir" },
        { .string = ctu_realpath(path) }
    };

    record_t record = {
        .layout = &HEADER_LAYOUT,
        .values = values
    };

    header_t header = { 
        .reports = reports,
        .format = &GLOBAL,
        .path = path,
        .header = record,
        .submagic = HLIR_SUBMAGIC,
        .semver = HLIR_VERSION
    };

    data_t data;
    begin_save(&data, header);

    save_node(&data, module);

    end_save(&data);
}
