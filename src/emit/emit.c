#include "cthulhu/emit/emit.h"

#include "cJSON.h"

static cJSON *emit_span(const node_t *node) {
    cJSON *span = cJSON_CreateObject();
    cJSON_AddNumberToObject(span, "first-line", node->where.first_line);
    cJSON_AddNumberToObject(span, "first-column", node->where.first_column);
    cJSON_AddNumberToObject(span, "last-line", node->where.last_line);
    cJSON_AddNumberToObject(span, "last-column", node->where.last_column);
    return span;
}

static cJSON *emit_value(reports_t *reports, const value_t *value) {
    UNUSED(reports);
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", value->type->name);
    return json;
}

static cJSON *emit_operand(reports_t *reports, operand_t op) {
    cJSON *json = cJSON_CreateObject();

    switch (op.type) {
    case OPERAND_VREG:
        cJSON_AddStringToObject(json, "type", "vreg");
        cJSON_AddNumberToObject(json, "vreg", op.vreg);
        break;
    case OPERAND_VALUE:
        cJSON_AddStringToObject(json, "type", "value");
        cJSON_AddItemToObject(json, "value", emit_value(reports, op.value));
        break;
    case OPERAND_BLOCK:
        cJSON_AddStringToObject(json, "type", "block");
        cJSON_AddStringToObject(json, "name", op.block->name);
        break;
    default:
        report(reports, INTERNAL, NULL, "unknown operand type %d", op.type);
        cJSON_AddStringToObject(json, "type", "unknown");
        break;
    }

    return json;
}

static cJSON *emit_load(reports_t *reports, step_t step) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "load");
    cJSON_AddItemToObject(result, "value", emit_operand(reports, step.value));
    return result;
}

static cJSON *emit_binary(reports_t *reports, step_t step) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "binary");
    cJSON_AddItemToObject(result, "lhs", emit_operand(reports, step.lhs));
    cJSON_AddItemToObject(result, "rhs", emit_operand(reports, step.rhs));
    cJSON_AddStringToObject(result, "binary", binary_name(step.binary));
    return result;
}

static cJSON *emit_return(reports_t *reports, step_t step) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "return");
    cJSON_AddItemToObject(result, "value", emit_operand(reports, step.value));
    return result;
}

static cJSON *emit_step(reports_t *reports, step_t step) {
    cJSON *result;
    switch (step.type) {
    case OP_LOAD: 
        result = emit_load(reports, step);
        break;
    case OP_BINARY: 
        result = emit_binary(reports, step);
        break;
    case OP_RETURN: 
        result = emit_return(reports, step);
        break;

    default:
        report(reports, INTERNAL, step.node, "unknown step kind %d", step.type);
        result = cJSON_CreateObject();
        break;
    }

    cJSON_AddItemToObject(result, "span", emit_span(step.node));
    return result;
}

static cJSON *emit_block(reports_t *reports, block_t *block) {
    cJSON *json = cJSON_CreateObject();
    cJSON *body = cJSON_CreateArray();
    cJSON_AddStringToObject(json, "name", block->name);

    for (size_t i = 0; i < block->length; i++) {
        cJSON *step = emit_step(reports, block->steps[i]);
        cJSON_AddItemToArray(body, step);
    }

    cJSON_AddItemToObject(json, "body", body);
    return json;
}

static cJSON *emit_vector(reports_t *reports, vector_t *vec) {
    cJSON *globals = cJSON_CreateArray();

    for (size_t i = 0; i < vector_len(vec); i++) {
        block_t *global = vector_get(vec, i);
        cJSON *block = emit_block(reports, global);
        cJSON_AddItemToArray(globals, block);
    }

    return globals;
}

void emit_module(reports_t *reports, module_t *mod) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", mod->name);
    cJSON_AddStringToObject(root, "path", mod->source->path);
    cJSON_AddStringToObject(root, "lang", mod->source->language);

    cJSON *globals = emit_vector(reports, mod->globals);
    cJSON_AddItemToObject(root, "globals", globals);

    cJSON *functions = emit_vector(reports, mod->functions);
    cJSON_AddItemToObject(root, "functions", functions);

    report(reports, NOTE, NULL, "%s", cJSON_Print(root));
}
