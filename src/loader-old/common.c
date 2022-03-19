#include "common.h"

size_t get_node_size(reports_t *reports, uint8_t type) {
    switch (type) {
    case HLIR_DIGIT_LITERAL: return sizeof(digit_node_t);
    case HLIR_BOOL_LITERAL: return sizeof(bool_node_t);
    case HLIR_STRING_LITERAL: return sizeof(string_node_t);
    
    case HLIR_NAME:
    case HLIR_UNARY:
    case HLIR_BINARY:
    case HLIR_COMPARE:
    case HLIR_CALL:
    case HLIR_ARRAY_INIT: 
        return 0;

    case HLIR_STMTS:
    case HLIR_BRANCH:
    case HLIR_LOOP:
    case HLIR_ASSIGN:
        return 0;

    case HLIR_STRUCT:
    case HLIR_UNION:
        return 0;

    case HLIR_DIGIT: return sizeof(digit_type_node_t);

    case HLIR_BOOL:
    case HLIR_STRING:
    case HLIR_VOID:
    case HLIR_CLOSURE:
    case HLIR_POINTER:
    case HLIR_ARRAY:
        return 0;

    case HLIR_TYPE: return sizeof(metatype_node_t);

    case HLIR_ALIAS:
        return 0;

    case HLIR_FORWARD:
    case HLIR_FUNCTION:
        return 0;

    case HLIR_VALUE: return sizeof(value_node_t);
    case HLIR_MODULE: return sizeof(module_node_t);

    case HLIR_FIELD:
        return 0;

    case HLIR_ERROR:
        return 0;

    default:
        ctu_assert(reports, "get-node-count called with invalid type %d", type);
        return 0;
    }
}

size_t get_node_count(reports_t *reports, length_t bytes, uint8_t type) {
    size_t size = get_node_size(reports, type);
    if (size == 0) { return 0; }

    if (bytes % size != 0) {
        message_t *id = report(reports, ERROR, NULL, "corrupted node size");
        report_note(id, "expected size to be a multiple of %zu", size);
    }

    return bytes / size;
}
