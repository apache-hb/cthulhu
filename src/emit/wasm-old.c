#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/util/report.h"

#include <stdio.h>

typedef struct
{
    reports_t *reports;
    stream_t *stream;
    size_t depth;
} wasm_t;

static void wasm_indent(wasm_t *wasm)
{
    wasm->depth += 1;
}

static void wasm_dedent(wasm_t *wasm)
{
    wasm->depth -= 1;
}

static void wasm_begin(wasm_t *wasm, const char *str)
{
    for (size_t i = 0; i < wasm->depth; i++)
    {
        stream_write(wasm->stream, "  ");
    }

    stream_write(wasm->stream, "(");
    stream_write(wasm->stream, str);
    stream_write(wasm->stream, "\n");

    wasm_indent(wasm);
}

static void wasm_end(wasm_t *wasm)
{
    wasm_dedent(wasm);

    for (size_t i = 0; i < wasm->depth; i++)
    {
        stream_write(wasm->stream, "  ");
    }

    stream_write(wasm->stream, ")\n");
}

static void wasm_write(wasm_t *wasm, const char *str)
{
    for (size_t i = 0; i < wasm->depth; i++)
    {
        stream_write(wasm->stream, "  ");
    }

    stream_write(wasm->stream, str);
    stream_write(wasm->stream, "\n");
}

static const char *get_width(wasm_t *wasm, digit_t digit)
{
    switch (digit)
    {
    case DIGIT_CHAR:
    case DIGIT_SHORT:
    case DIGIT_INT:
        return "32";

    case DIGIT_LONG:
    case DIGIT_SIZE: // TODO: wrong wrong wrong wrong
    case DIGIT_PTR:  //       awfully wrong, wasm has memory segments
                     //       and we need to respect that in our type system
        return "64";

    default:
        ctu_assert(wasm->reports, "unhandled digit %d", digit);
        return "error";
    }
}

static char *emit_wasm_digit(wasm_t *wasm, const hlir_t *node)
{
    const char *sign = node->sign == SIGN_UNSIGNED ? "u" : "i";
    const char *width = get_width(wasm, node->width);

    return format("%s%s", sign, width);
}

static char *emit_wasm_type(wasm_t *wasm, const hlir_t *node)
{
    switch (node->type)
    {
    case HLIR_DIGIT:
        return emit_wasm_digit(wasm, node);
    default:
        ctu_assert(wasm->reports, "emit-type unknown type %d", node->type);
        return ctu_strdup("error");
    }
}

static void emit_node(wasm_t *wasm, const hlir_t *node);

static void emit_module_node(wasm_t *wasm, const hlir_t *node)
{
    wasm_begin(wasm, "module");

    for (size_t i = 0; i < vector_len(node->globals); i++)
    {
        emit_node(wasm, vector_get(node->globals, i));
    }

    for (size_t i = 0; i < vector_len(node->functions); i++)
    {
        emit_node(wasm, vector_get(node->functions, i));
    }

    wasm_end(wasm);
}

static void emit_function_node(wasm_t *wasm, const hlir_t *node)
{
    if (node->body == NULL)
    {
        return;
    }

    wasm_begin(wasm, format("func $%s", get_hlir_name(node)));

    for (size_t i = 0; i < vector_len(node->params); i++)
    {
        const hlir_t *param = vector_get(node->params, i);
        char *type = emit_wasm_type(wasm, param);
        wasm_write(wasm, format("(param %s)", type));
    }

    for (size_t i = 0; i < vector_len(node->locals); i++)
    {
        const hlir_t *local = vector_get(node->locals, i);
        char *type = emit_wasm_type(wasm, get_hlir_type(local));
        wasm_write(wasm, format("(local $%s %s)", get_hlir_name(local), type));
    }

    if (!hlir_is(node->result, HLIR_VOID))
    {
        char *result = emit_wasm_type(wasm, node->result);
        wasm_write(wasm, format("(result %s)", result));
    }

    emit_node(wasm, node->body);

    wasm_end(wasm);
}

static void emit_stmts_node(wasm_t *wasm, const hlir_t *node)
{
    for (size_t i = 0; i < vector_len(node->stmts); i++)
    {
        emit_node(wasm, vector_get(node->stmts, i));
    }
}

static void emit_assign_node(wasm_t *wasm, const hlir_t *node)
{
    emit_node(wasm, node->src);

    hlir_kind_t kind = get_hlir_kind(node->dst);

    switch (kind)
    {
    case HLIR_GLOBAL:
        wasm_write(wasm, format("global.set $%s", get_hlir_name(node->dst)));
        break;

    case HLIR_LOCAL:
        wasm_write(wasm, format("local.set $%s", get_hlir_name(node->dst)));
        break;

    default:
        ctu_assert(wasm->reports, "unhandled assign to %s", hlir_kind_to_string(kind));
        break;
    }
}

static void emit_digit_literal_node(wasm_t *wasm, const hlir_t *node)
{
    char *str = emit_wasm_type(wasm, get_hlir_type(node));
    wasm_write(wasm, format("%s.const %s", str, mpz_get_str(NULL, 10, node->digit)));
}

static void emit_value_node(wasm_t *wasm, const hlir_t *node)
{
    const char *name = get_hlir_name(node);
    char *type = emit_wasm_type(wasm, get_hlir_type(node));
    wasm_begin(wasm, format("global $%s %s", name, type));

    if (node->value != NULL)
    {
        emit_node(wasm, node->value);
    }
    else
    {
        wasm_write(wasm, format("(%s.const 0)", type));
    }

    wasm_end(wasm);
}

static void emit_loop_node(wasm_t *wasm, const hlir_t *node)
{
    emit_node(wasm, node->cond);
    wasm_write(wasm, "br_if 0");

    wasm_begin(wasm, "loop");
    emit_node(wasm, node->then);

    emit_node(wasm, node->cond);
    wasm_write(wasm, "br_if 0");

    wasm_end(wasm);
}

static const char *kCompareNames[COMPARE_TOTAL] = {
    [COMPARE_EQ] = "eq",    [COMPARE_NEQ] = "ne",  [COMPARE_LT] = "lt_s",
    [COMPARE_LTE] = "le_s", [COMPARE_GT] = "gt_s", [COMPARE_GTE] = "ge_s"};

static const char *kBinaryNames[BINARY_TOTAL] = {[BINARY_ADD] = "add",    [BINARY_SUB] = "sub",   [BINARY_MUL] = "mul",
                                                 [BINARY_DIV] = "div_s",  [BINARY_REM] = "rem_s",

                                                 [BINARY_BITAND] = "and", [BINARY_BITOR] = "or",  [BINARY_XOR] = "xor",

                                                 [BINARY_SHL] = "shl",    [BINARY_SHR] = "shr_s"};

static void emit_compare_node(wasm_t *wasm, const hlir_t *node)
{
    emit_node(wasm, node->lhs);
    emit_node(wasm, node->rhs);

    char *type = emit_wasm_type(wasm, get_hlir_type(node->lhs)); // TODO: check both sides
    wasm_write(wasm, format("%s.%s", type, kCompareNames[node->compare]));
}

static void emit_binary_node(wasm_t *wasm, const hlir_t *node)
{
    emit_node(wasm, node->lhs);
    emit_node(wasm, node->rhs);

    char *type = emit_wasm_type(wasm, get_hlir_type(node->lhs)); // TODO: check both sides
    wasm_write(wasm, format("%s.%s", type, kBinaryNames[node->binary]));
}

static void emit_name_node(wasm_t *wasm, const hlir_t *node)
{
    const hlir_t *val = node->read;
    switch (get_hlir_kind(val))
    {
    case HLIR_GLOBAL:
        wasm_write(wasm, format("global.get $%s", get_hlir_name(val)));
        break;

    case HLIR_LOCAL:
        wasm_write(wasm, format("local.get $%s", get_hlir_name(val)));
        break;

    default:
        ctu_assert(wasm->reports, "unhandled name node %s", hlir_kind_to_string(get_hlir_kind(val)));
        break;
    }
}

static void emit_call_node(wasm_t *wasm, const hlir_t *node)
{
    for (size_t i = 0; i < vector_len(node->args); i++)
    {
        emit_node(wasm, vector_get(node->args, i));
    }

    if (get_hlir_kind(node->call) == HLIR_FUNCTION)
    {
        wasm_write(wasm, format("call $%s", get_hlir_name(node->call)));
    }
    else
    {
        ctu_assert(wasm->reports, "unhandled call node %s", hlir_kind_to_string(get_hlir_kind(node->call)));
    }
}

static void emit_branch_node(wasm_t *wasm, const hlir_t *node)
{
    emit_node(wasm, node->cond);
    wasm_write(wasm, "br_if 0");

    wasm_begin(wasm, "then");
    emit_node(wasm, node->then);
    wasm_end(wasm);

    if (node->other != NULL)
    {
        wasm_begin(wasm, "else");
        emit_node(wasm, node->other);
        wasm_end(wasm);
    }
}

static void emit_node(wasm_t *wasm, const hlir_t *node)
{
    hlir_kind_t kind = get_hlir_kind(node);
    switch (kind)
    {
    case HLIR_MODULE:
        emit_module_node(wasm, node);
        break;

    case HLIR_FUNCTION:
        emit_function_node(wasm, node);
        break;

    case HLIR_STMTS:
        emit_stmts_node(wasm, node);
        break;

    case HLIR_ASSIGN:
        emit_assign_node(wasm, node);
        break;

    case HLIR_DIGIT_LITERAL:
        emit_digit_literal_node(wasm, node);
        break;

    case HLIR_GLOBAL:
        emit_value_node(wasm, node);
        break;

    case HLIR_LOOP:
        emit_loop_node(wasm, node);
        break;

    case HLIR_COMPARE:
        emit_compare_node(wasm, node);
        break;

    case HLIR_BINARY:
        emit_binary_node(wasm, node);
        break;

    case HLIR_NAME:
        emit_name_node(wasm, node);
        break;

    case HLIR_CALL:
        emit_call_node(wasm, node);
        break;

    case HLIR_BRANCH:
        emit_branch_node(wasm, node);
        break;

    default:
        ctu_assert(wasm->reports, "unhandled %s node", hlir_kind_to_string(kind));
        break;
    }
}

void wasm_emit_tree(reports_t *reports, const hlir_t *hlir)
{
    wasm_t wasm = {reports, stream_new(0x1000), 0};
    emit_node(&wasm, hlir);

    printf("%s", stream_data(wasm.stream));
}
