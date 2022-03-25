#include "cthulhu/loader/hlir.h"

#include "cthulhu/ast/compile.h"

///
/// layouts
///

typedef enum {
    SPAN_INDEX = HLIR_TOTAL,
    ATTRIBUTE_INDEX,
    LAYOUTS_TOTAL
} hlir_layouts_t;

typedef enum {
    SOURCE_PATH,
    SOURCE_TEXT,

    SOURCE_TOTAL
} source_kind_t;

// header data
enum { HEADER_LANGUAGE, HEADER_PATH, HEADER_SOURCE };
static const field_t HEADER_FIELDS[] = {
    [HEADER_LANGUAGE] = FIELD("language", FIELD_STRING),
    [HEADER_PATH] = FIELD("path", FIELD_STRING),
    [HEADER_SOURCE] = FIELD("source", FIELD_STRING)
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

enum { VALUE_SPAN, VALUE_ATTRIBS, VALUE_NAME, VALUE_TYPE, VALUE_INIT };
static const field_t VALUE_FIELDS[] = {
    [VALUE_SPAN] = FIELD("span", FIELD_REFERENCE),
    [VALUE_ATTRIBS] = FIELD("attribs", FIELD_REFERENCE),
    [VALUE_NAME] = FIELD("name", FIELD_STRING),
    [VALUE_TYPE] = FIELD("type", FIELD_REFERENCE),
    [VALUE_INIT] = FIELD("init", FIELD_REFERENCE)
};
static const layout_t VALUE_LAYOUT = LAYOUT("value", VALUE_FIELDS);

enum { ATTRIB_LINKAGE };
static const field_t ATTRIB_FIELDS[] = {
    [ATTRIB_LINKAGE] = FIELD("linkage", FIELD_INT)
};
static const layout_t ATTRIB_LAYOUT = LAYOUT("attributes", ATTRIB_FIELDS);

enum { DIGIT_LITERAL_SPAN, DIGIT_LITERAL_TYPE, DIGIT_LITERAL_VALUE };
static const field_t DIGIT_LITERAL_FIELDS[] = {
    [DIGIT_LITERAL_SPAN] = FIELD("span", FIELD_REFERENCE),
    [DIGIT_LITERAL_TYPE] = FIELD("type", FIELD_REFERENCE),
    [DIGIT_LITERAL_VALUE] = FIELD("value", FIELD_INT)
};
static const layout_t DIGIT_LITERAL_LAYOUT = LAYOUT("digit-literal", DIGIT_LITERAL_FIELDS);

enum { DIGIT_SPAN, DIGIT_NAME, DIGIT_SIGN, DIGIT_WIDTH };
static const field_t DIGIT_FIELDS[] = {
    [DIGIT_SPAN] = FIELD("span", FIELD_REFERENCE),
    [DIGIT_NAME] = FIELD("name", FIELD_STRING),
    [DIGIT_SIGN] = FIELD("sign", FIELD_INT),
    [DIGIT_WIDTH] = FIELD("width", FIELD_INT)
};
static const layout_t DIGIT_LAYOUT = LAYOUT("digit", DIGIT_FIELDS);

#if 0
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
    [ATTRIBUTE_INDEX] = ATTRIB_LAYOUT,

    [HLIR_MODULE] = MODULE_LAYOUT,
    [HLIR_VALUE] = VALUE_LAYOUT,

    [HLIR_DIGIT_LITERAL] = DIGIT_LITERAL_LAYOUT,

    [HLIR_DIGIT] = DIGIT_LAYOUT

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
    reports_t *reports;
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

static hlir_attributes_t *load_attributes(load_t *load, index_t index) {
    value_t values[FIELDLEN(ATTRIB_FIELDS)];
    read_entry(load->data, index, values);

    return hlir_new_attributes(get_int(values[ATTRIB_LINKAGE]));
}

static hlir_t *load_node(load_t *load, index_t index, const char *trace);

static vector_t *load_array(load_t *load, array_t array) {
    size_t len = array.length;
    index_t *indices = ctu_malloc(sizeof(index_t) * len);
    vector_t *vec = vector_of(len);

    read_array(load->data, array, indices);

    for (size_t i = 0; i < len; i++) {
        hlir_t *hlir = load_node(load, indices[i], format("array@%zu[%zu + %zu]", array.offset, array.length, i));
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

static hlir_t *load_value_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(VALUE_FIELDS)];
    read_entry(load->data, index, values);

    const node_t *node = load_span(load, get_reference(values[VALUE_SPAN]));
    const char *name = get_string(values[VALUE_NAME]);
    hlir_attributes_t *attributes = load_attributes(load, get_reference(values[VALUE_ATTRIBS]));
    hlir_t *type = load_node(load, get_reference(values[VALUE_TYPE]), "load value type");
    hlir_t *init = load_node(load, get_reference(values[VALUE_INIT]), "load value init");

    hlir_t *hlir = hlir_new_value(node, name, type);
    hlir_build_value(hlir, init);
    hlir_set_attributes(hlir, attributes);

    return hlir;
}

static hlir_t *load_digit_literal_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(DIGIT_LITERAL_FIELDS)];
    read_entry(load->data, index, values);

    const node_t *node = load_span(load, get_reference(values[DIGIT_LITERAL_SPAN]));
    const hlir_t *type = load_node(load, get_reference(values[DIGIT_LITERAL_TYPE]), "load digit literal type");
    
    mpz_t digit;
    get_digit(digit, values[DIGIT_LITERAL_VALUE]);

    hlir_t *hlir = hlir_digit_literal(node, type, digit);

    return hlir;
}

static hlir_t *load_digit_node(load_t *load, index_t index) {
    value_t values[FIELDLEN(DIGIT_FIELDS)];
    read_entry(load->data, index, values);

    const node_t *node = load_span(load, get_reference(values[DIGIT_SPAN]));
    const char *name = get_string(values[DIGIT_NAME]);
    sign_t sign = get_int(values[DIGIT_SIGN]);
    digit_t width = get_int(values[DIGIT_WIDTH]);

    hlir_t *hlir = hlir_digit(node, name, width, sign);

    return hlir;
}

static hlir_t *load_node(load_t *load, index_t index, const char *trace) {
    switch (index.type) {
    case HLIR_MODULE:
        return load_module_node(load, index);
    case HLIR_VALUE:
        return load_value_node(load, index);

    case HLIR_DIGIT_LITERAL:
        return load_digit_literal_node(load, index);

    case HLIR_DIGIT:    
        return load_digit_node(load, index);

    default:
        ctu_assert(load->reports, "loading unknown node type %d due to %s", index.type, trace);
        return NULL;
    }
}

static scan_t make_scanner(reports_t *reports, const char *lang, const char *path, const char *source) {
    if (source != NULL) {
        return scan_string(reports, lang, path, source);
    }

    file_t file = ctu_fopen(path, "r");
    file_t *fp = BOX(file);

    return scan_file(reports, lang, fp);
}

hlir_t *load_module(reports_t *reports, const char *path) {
    value_t values[FIELDLEN(HEADER_FIELDS)];
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
    const char *where = get_string(data.header.header.values[HEADER_PATH]);
    const char *source = get_string(data.header.header.values[HEADER_SOURCE]);

    logverbose("loading(lang=%s, path=%s, source=%s)", language, where, source != NULL ? "embedded" : "external");

    scan_t scan = make_scanner(reports, language, where, source);

    load_t load = {
        .reports = reports,
        .scan = BOX(scan),
        .data = &data
    };
    index_t index = { .type = HLIR_MODULE };
    hlir_t *hlir = load_node(&load, index, "root load");

    end_load(&data);

    return hlir;
}

///
/// hlir saving
///

static index_t save_span(data_t *data, const node_t *node) {
    if (node == NULL) { return NULL_INDEX; }

    value_t values[FIELDLEN(SPAN_FIELDS)] = {
        [SPAN_FIRST_LINE] = int_value(node->where.first_line),
        [SPAN_FIRST_COLUMN] = int_value(node->where.first_column),
        [SPAN_LAST_LINE] = int_value(node->where.last_line),
        [SPAN_LAST_COLUMN] = int_value(node->where.last_column)
    };
    
    return write_entry(data, SPAN_INDEX, values);
}

static index_t save_attributes(data_t *data, const hlir_attributes_t *attributes) {
    value_t values[FIELDLEN(ATTRIB_FIELDS)] = {
        [ATTRIB_LINKAGE] = int_value(attributes->linkage)
    };

    return write_entry(data, ATTRIBUTE_INDEX, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir);

static array_t save_array(data_t *data, vector_t *vec) {
    size_t len = vector_len(vec);
    index_t *indices = ctu_malloc(sizeof(index_t) * len);

    for (size_t i = 0; i < len; i++) {
        indices[i] = save_node(data, vector_get(vec, i));
    }

    array_t array = write_array(data, indices, len);

    return array;
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

static index_t save_value_node(data_t *data, const hlir_t *hlir) {
    index_t span = save_span(data, hlir->node);
    index_t attribs = save_attributes(data, hlir->attributes);
    index_t type = save_node(data, typeof_hlir(hlir));
    index_t init = save_node(data, hlir->value);
    
    value_t values[] = {
        [VALUE_SPAN] = reference_value(span),
        [VALUE_NAME] = string_value(hlir->name),
        [VALUE_ATTRIBS] = reference_value(attribs),
        [VALUE_TYPE] = reference_value(type),
        [VALUE_INIT] = reference_value(init)
    };
    
    return write_entry(data, HLIR_VALUE, values);
}

static index_t save_digit_literal_node(data_t *data, const hlir_t *hlir) {
    index_t span = save_span(data, hlir->node);
    index_t type = save_node(data, typeof_hlir(hlir));
    
    value_t values[] = {
        [DIGIT_LITERAL_SPAN] = reference_value(span),
        [DIGIT_LITERAL_TYPE] = reference_value(type),
        [DIGIT_LITERAL_VALUE] = digit_value(hlir->digit)
    };

    return write_entry(data, HLIR_DIGIT_LITERAL, values);
}

static index_t save_digit_node(data_t *data, const hlir_t *hlir) {
    index_t span = save_span(data, hlir->node);
    
    value_t values[] = {
        [DIGIT_SPAN] = reference_value(span),
        [DIGIT_NAME] = string_value(hlir->name),
        [DIGIT_SIGN] = int_value(hlir->sign),
        [DIGIT_WIDTH] = int_value(hlir->width)
    };

    return write_entry(data, HLIR_DIGIT, values);
}

static index_t save_node(data_t *data, const hlir_t *hlir) {
    switch (hlir->type) {
    case HLIR_MODULE:
        return save_module_node(data, hlir);
    case HLIR_VALUE:
        return save_value_node(data, hlir);

    case HLIR_DIGIT_LITERAL:
        return save_digit_literal_node(data, hlir);

    case HLIR_DIGIT:
        return save_digit_node(data, hlir);

    case HLIR_TYPE:
        return NULL_INDEX;

    default:
        ctu_assert(data->header.reports, "saving unknown node type %d", hlir->type);
        return NULL_INDEX;
    }
}

void save_module(reports_t *reports, save_settings_t *settings, hlir_t *module, const char *path) {
    scan_t *scan = module->node->scan;

    value_t values[] = {
        [HEADER_LANGUAGE] = string_value(scan->language),
        [HEADER_PATH] = string_value(scan->path),
        [HEADER_SOURCE] = string_value(NULL)
    };

    if (settings->embed_source) {
        values[HEADER_SOURCE] = string_value(scan_text(scan));
    }

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
