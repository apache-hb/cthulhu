#include "cthulhu/ssa/debug.h"

static char *value_debug(const value_t *value) {
    if (value == NULL || value->type == NULL) {
        return "null";
    }

    if (is_string(value->type)) {
        return format("\"%s\"", strnorm(value->string));
    }

    if (is_integer(value->type)) {
        return mpz_get_str(NULL, 10, value->digit);
    }
    
    return NULL;
}

static char *operand_debug(operand_t operand) {
    switch (operand.type) {
    case OPERAND_EMPTY: return format("empty");
    case OPERAND_VREG: return format("%%%zu", operand.vreg);
    case OPERAND_BLOCK: return format("&%s", operand.block->name);
    case OPERAND_VALUE: return value_debug(operand.value);
    default: return format("<%d>", operand.type);
    }
}

static void step_debug(size_t idx, step_t step) {
    switch (step.type) {
    case OP_LOAD: 
        printf("  %%%zu = load %s\n", idx, operand_debug(step.value));
        break;
    case OP_CALL:
        printf("  %%%zu = call %s(", idx, operand_debug(step.call));
        for (size_t i = 0; i < step.total; i++) {
            printf("%s", operand_debug(step.operands[i]));
            if (i != step.total - 1) {
                printf(", ");
            }
        }
        printf(")\n");
        break;
    case OP_BINARY:
        printf("  %%%zu = %s %s %s\n", idx, binary_name(step.binary), operand_debug(step.lhs), operand_debug(step.rhs));
        break;
    case OP_RETURN:
        printf("  ret %s\n", operand_debug(step.value));
        break;
    case OP_STORE:
        printf("  store %s %s\n", operand_debug(step.dst), operand_debug(step.src));
        break;
    default:
        printf("  %%%zu = <%d>\n", idx, step.type);
        break;
    }
}

static void block_debug(block_t *block) {
    printf("block %s {\n", block->name);
    for (size_t i = 0; i < block->length; i++) {
        step_debug(i, block->steps[i]);
    }
    printf("}\n");
}

void ssa_debug(module_t *mod) {
    printf("module %s;\n", mod->name);
    printf("; globals\n");
    for (size_t i = 0; i < vector_len(mod->globals); i++) {
        block_t *global = vector_get(mod->globals, i);
        block_debug(global);
    }
    printf("; functions\n");
    for (size_t i = 0; i < vector_len(mod->functions); i++) {
        block_t *func = vector_get(mod->functions, i);
        block_debug(func);
    }
}
