#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/query.h"
#include "cthulhu/util/str.h"

#include <stdio.h>

static const char *expr_as_string(const hlir_t *expr)
{
    hlir_kind_t kind = get_hlir_kind(expr);
    switch (kind)
    {
    case HLIR_DIGIT_LITERAL:
        return mpz_get_str(NULL, 10, expr->digit);

    case HLIR_BOOL_LITERAL:
        return expr->boolean ? "true" : "false";

    case HLIR_STRING_LITERAL:
        return format("\"%s\"", str_normalizen(expr->string, expr->stringLength));

    default:
        return hlir_kind_to_string(kind);
    }
}

static const char *type_as_string(const hlir_t *type)
{
    hlir_kind_t kind = get_hlir_kind(type);
    switch (kind)
    {
    case HLIR_BOOL:
        return "bool";
    case HLIR_DIGIT:
        return format("digit(%s, %s)", hlir_sign_to_string(type->sign), hlir_digit_to_string(type->width));
    case HLIR_STRING:
        return "string";
    case HLIR_ARRAY:
        return format("%s[%s]", type_as_string(type->element), expr_as_string(type->length)); // TODO: length
    case HLIR_POINTER:
        return format("%s%s", type_as_string(type->ptr), type->indexable ? "[*]" : "*");

    default:
        return hlir_kind_to_string(kind);
    }
}

static const char *operand_to_string(operand_t op)
{
    switch (op.kind)
    {
    case OPERAND_EMPTY:
        return "empty";

    case OPERAND_VREG:
        return format("%%%zu", op.vreg);

    case OPERAND_CONST:
        return format("%s %s", type_as_string(get_hlir_type(op.value)), expr_as_string(op.value));

    default:
        return "???";
    }
}

static const char *step_to_string(step_list_t list, size_t idx)
{
    step_t step = list.steps[idx];

    switch (step.op)
    {
    case OP_EMPTY:
        return "empty";

    case OP_RETURN:
        return format("ret %s", operand_to_string(step.operand));

    case OP_CONST:
        return format("%%%zu = const %s", idx, operand_to_string(step.value));

    default:
        return "unknown";
    }
}

void ssa_print(const ssa_t *ssa)
{
    printf("module %s:\n", ssa->name);

    printf("  globals:\n");

    map_iter_t globalIter = map_iter(ssa->globals);

    while (map_has_next(&globalIter))
    {
        map_entry_t entry = map_next(&globalIter);
        
        const char *name = entry.key;
        const flow_t *flow = entry.value;

        const char *type = type_as_string(flow->type);
        printf("    global %s: %s {\n", name, type);

        for (size_t i = 0; i < flow->steps.used; i++)
        {
            printf("      %s\n", step_to_string(flow->steps, i));
        }

        printf("    }\n");
    }
}
