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
    case OPERAND_LABEL:
        cJSON_AddStringToObject(json, "type", "label");
        cJSON_AddNumberToObject(json, "name", op.label);
        break;
    case OPERAND_EMPTY:
        return NULL;
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

static cJSON *emit_store(reports_t *reports, step_t step) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "store");
    cJSON_AddItemToObject(result, "dst", emit_operand(reports, step.dst));
    cJSON_AddItemToObject(result, "src", emit_operand(reports, step.src));
    return result;
}

static cJSON *emit_call(reports_t *reports, step_t step) {
    cJSON *args = cJSON_CreateArray();
    for (size_t i = 0; i < step.total; i++) {
        cJSON_AddItemToArray(args, emit_operand(reports, step.operands[i]));
    }
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "call");
    cJSON_AddItemToObject(result, "call", emit_operand(reports, step.call));
    cJSON_AddItemToObject(result, "args", args);
    return result;
}

static cJSON *emit_label() {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "label");
    return result;
}

static cJSON *emit_branch(reports_t *reports, step_t step) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "branch");
    cJSON_AddItemToObject(result, "cond", emit_operand(reports, step.cond));
    cJSON_AddItemToObject(result, "true", emit_operand(reports, step.then));
    cJSON_AddItemToObject(result, "false", emit_operand(reports, step.other));
    return result;
}

static cJSON *emit_compare(reports_t *reports, step_t step) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "compare");
    cJSON_AddItemToObject(result, "lhs", emit_operand(reports, step.lhs));
    cJSON_AddItemToObject(result, "rhs", emit_operand(reports, step.rhs));
    cJSON_AddStringToObject(result, "compare", compare_name(step.compare));
    return result;
}

static cJSON *emit_jump(reports_t *reports, step_t step) {
    cJSON *result = cJSON_CreateObject();
    cJSON_AddStringToObject(result, "op", "jump");
    cJSON_AddItemToObject(result, "dst", emit_operand(reports, step.dst));
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
    case OP_STORE:
        result = emit_store(reports, step);
        break;
    case OP_CALL:
        result = emit_call(reports, step);
        break;
    case OP_COMPARE:
        result = emit_compare(reports, step);
        break;
    case OP_LABEL:
        result = emit_label();
        break;
    case OP_BRANCH:
        result = emit_branch(reports, step);
        break;
    case OP_JMP:
        result = emit_jump(reports, step);
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
