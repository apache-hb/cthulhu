#include "cthulhu/loader/hlir.h"

static const field_t HEADER_FIELDS[] = {
    FIELD("language", FIELD_STRING),
    FIELD("path", FIELD_STRING)
};

static const layout_t HEADER = LAYOUT("header", HEADER_FIELDS);

static const field_t MODULE_FIELDS[] = {
    FIELD("span", FIELD_REFERENCE),
    FIELD("name", FIELD_STRING),
    FIELD("globals", FIELD_ARRAY)
};

static const field_t DIGIT_FIELDS[] = {
    FIELD("span", FIELD_REFERENCE),
    FIELD("type", FIELD_REFERENCE),
    FIELD("init", FIELD_INT)
};

static const field_t STRING_FIELDS[] = {
    FIELD("span", FIELD_REFERENCE),
    FIELD("type", FIELD_REFERENCE),
    FIELD("init", FIELD_STRING)
};

static const layout_t TYPES[HLIR_TOTAL] = {
    [HLIR_MODULE] = LAYOUT("module", MODULE_FIELDS),
    [HLIR_DIGIT_LITERAL] = LAYOUT("digit", DIGIT_FIELDS),
    [HLIR_STRING_LITERAL] = LAYOUT("string", STRING_FIELDS)
};

static const format_t GLOBAL = FORMAT(HEADER, TYPES);

#define HLIR_SUBMAGIC 0xAAAA
#define HLIR_VERSION NEW_VERSION(0, 0, 1)

static index_t save_node(data_t *data, hlir_t *node);

static index_t save_module_node(data_t *data, hlir_t *node) {

}

static index_t save_value_node(data_t *data, hlir_t *node) {

}

static index_t save_node(data_t *data, hlir_t *node) {
    index_t index = { 0, 0 };
    switch (node->type) {
    case HLIR_MODULE:
        return save_module_node(data, node);
    case HLIR_VALUE:
        return save_value_node(data, node);

    default:
        return index;
    }
}

hlir_t *load_module(reports_t *reports, const char *path) {
    value_t values[] = {
        { .string = "hlir" },
        { .string = path }
    };

    record_t record = {
        .layout = &HEADER,
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
    
    end_load(&data);

    return NULL;
}

void save_module(reports_t *reports, hlir_t *module, const char *path) {
    value_t values[] = {
        { .string = "hlir" },
        { .string = path }
    };

    record_t record = {
        .layout = &HEADER,
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

    end_save(&data);
}
