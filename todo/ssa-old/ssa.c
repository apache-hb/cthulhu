#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/util.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/vector.h"
#include "std/str.h"
#include "std/set.h"

#include "report/report.h"

#include "common.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    reports_t *reports;

    map_t *aggregates;

    map_t *globals;
    map_t *functions;

    set_t *strings;

    map_t *importedSymbols; // map_t<const char *, const h2_t *>

    map_t *currentLocals; // map_t<const h2_t *, size_t> // map of local variables to their index in the locals vector
    map_t *currentParams; // map_t<const h2_t *, size_t> // map of parameters to their index in the params vector

    ssa_block_t *currentBlock;
    ssa_flow_t *currentFlow;
    size_t stepIdx;
    size_t blockIdx;

    vector_t *namePath; // vector_t<const char *>

    const ssa_type_t *emptyType;
    const ssa_type_t *stubType;
} ssa_t;

static ssa_block_t *block_new(const char *id) 
{
    ssa_block_t *block = ctu_malloc(sizeof(ssa_block_t));
    block->steps = vector_new(16);
    block->id = id;
    return block;
}

static ssa_block_t *block_gen(ssa_t *ssa, const char *id)
{
    return block_new(format("%s%zu", id, ssa->blockIdx++));
}

static ssa_kind_t get_type_kind(ssa_t *ssa, const h2_t *hlir)
{
    h2_kind_t kind = h2_get_kind(hlir);
    switch (kind)
    {
    case eHlir2TypeDigit:
        return eTypeDigit;
    //case eHlirDecimal: return eTypeDecimal;
    case eHlir2TypeBool: return eTypeBool;
    case eHlir2TypeString: return eTypeString;
    //case eHlirPointer: return eTypePointer;
    case eHlir2TypeUnit: return eTypeUnit;
    case eHlir2TypeEmpty: return eTypeEmpty;

    case eHlir2TypeClosure:
        return eTypeSignature;

    //case eHlirOpaque: return eTypeOpaque;
   // case eHlirStruct: return eTypeStruct;
    //case eHlirUnion: return eTypeUnion;
    //case eHlirArray: return eTypeArray;

    default:
        report(ssa->reports, eInternal, h2_get_node(hlir), "no respective ssa type for %s", h2_to_string(hlir));
        return eTypeEmpty;
    }
}

typedef struct ssa_type_result_t {
    ssa_type_t *type;
    bool complete;
} ssa_type_result_t;

static bool should_add_aggregate(ssa_kind_t kind)
{
    return kind == eTypeStruct 
        || kind == eTypeUnion
        || kind == eTypePointer 
        || kind == eTypeSignature;
}

static ssa_type_result_t ssa_anytype_new(ssa_t *ssa, const void *key, const char *name, ssa_kind_t kind)
{
    if (key != NULL)
    {
        ssa_type_t *existing = map_get_ptr(ssa->aggregates, key);
        if (existing != NULL)
        {
            ssa_type_result_t result = { existing, true };
            return result;
        }
    }

    ssa_type_t *it = ctu_malloc(sizeof(ssa_type_t));
    it->kind = kind;
    it->name = name;

    if (should_add_aggregate(kind))
    {
        CTASSERT(key != NULL);
        map_set_ptr(ssa->aggregates, key, it);
    }
    
    ssa_type_result_t result = { it, false };

    return result;
}

static ssa_type_t *type_new(ssa_t *ssa, const h2_t *type)
{
    const h2_t *real = h2_follow_type(type);
    h2_kind_t kind = h2_get_kind(real);
    ssa_type_result_t result = ssa_anytype_new(ssa, real, h2_get_name(real), get_type_kind(ssa, real));
    if (result.complete)
    {
        return result.type;
    }

    ssa_type_t *it = result.type;

    switch (kind) 
    {
    case eHlir2TypeDigit:
        it->digit = real->digit;
        it->sign = real->sign;
        break;

    case eHlir2TypeClosure: {
        it->result = type_new(ssa, real->result);
        it->arity = real->arity;
        it->args = vector_of(vector_len(real->params));
        for (size_t i = 0; i < vector_len(real->params); i++) 
        {
            const h2_t *param = vector_get(real->params, i);
            const char *name = h2_get_name(param);
            ssa_type_t *arg = type_new(ssa, param);
            vector_set(it->args, i, ssa_param_new(name, arg));
        }
        break;
    }

    /*
    case eHlirStruct: 
    case eHlirUnion: {
        size_t len = vector_len(real->fields);
        it->fields = vector_of(len);
        if (len == 0)
        {
            vector_push(&it->fields, ssa_param_new("stub", ssa->stubType));
        }

        for (size_t i = 0; i < len; i++) 
        {
            const h2_t *field = vector_get(real->fields, i);
            const char *name = h2_get_name(field);
            ssa_type_t *type = type_new(ssa, h2_get_type(field));
            vector_set(it->fields, i, ssa_param_new(name, type));
        }
        break;
    }

    case eHlirPointer:
        it->ptr = type_new(ssa, real->ptr);
        break;

    case eHlirArray:
        CTASSERT(h2_is(real->length, eHlirDigitLiteral));
        it->arr = type_new(ssa, real->element);
        it->size = mpz_get_ui(real->length->digit);
        break;*/

    //case eHlirDecimal:
    //case eHlirOpaque:
    case eHlir2TypeBool:
    case eHlir2TypeUnit:
    case eHlir2TypeEmpty:
    case eHlir2TypeString: 
        break;

    default:
        report(ssa->reports, eInternal, h2_get_node(real), "no respective ssa type for %s", h2_to_string(type));
        break;
    }

    return it;
}

/**
 * @brief add a step to the current block and return the register it was assigned to
 */
static ssa_operand_t add_step(ssa_t *ssa, ssa_step_t step)
{
    ssa_step_t *ptr = BOX(step);
    if (ptr->id == NULL)
    {
        ptr->id = format("%zu", ssa->stepIdx++);
    }

    if (ptr->type == NULL)
    {
        ptr->type = ssa->emptyType;
    }

    vector_push(&ssa->currentBlock->steps, ptr);
    ssa_operand_t op = {
        .kind = eOperandReg,
        .vreg = ptr
    };

    return op;
}

static bool is_imported(h2_link_t link)
{
    return link == eLinkImport;
}

static const char *mangle_digit(digit_t width, sign_t sign)
{
    switch (width)
    {
    case eDigitChar:
        return sign == eSignSigned ? "c" : "h";
    case eDigitShort:
        return sign == eSignSigned ? "s" : "t";
    case eDigitInt:
        return sign == eSignSigned ? "i" : "j";
    case eDigitLong:
    case eDigitPtr:
        return sign == eSignSigned ? "x" : "y";
    case eDigitSize:
        return "m";
    default:
        return "";
    }
}

static const char *mangle_named_type(const h2_t *type)
{
    const char *name = h2_get_name(type);
    return format("%zu%s", strlen(name), name);
}

static const char *mangle_arg(ssa_t *ssa, const h2_t *param)
{
    const h2_t *type = h2_follow_type(param);
    h2_kind_t kind = h2_get_kind(type);
    switch (kind) 
    {
    //case eHlirPointer:
    //    return format("P%s", mangle_arg(ssa, type->ptr));
    case eHlir2TypeString: // TODO: rework string
        return "PKc";
    case eHlir2TypeBool:
        return "b";
    case eHlir2TypeDigit:
        return mangle_digit(type->digit, type->sign);
    // case eHlirStruct:
    // case eHlirUnion:
    //     return mangle_named_type(type);
    // case eHlirOpaque:
    //     return "Pv";
    case eHlir2TypeUnit:
        return "v";
    default:
        report(ssa->reports, eWarn, h2_get_node(param), "no mangle for %s", h2_to_string(param));
        return "";
    }
}

static vector_t *split_section(const char *section)
{
    size_t parts = str_count_any(section, "-./");
    if (parts == 0)
    {
        return NULL;
    }

    vector_t *result = vector_new(parts);

    const char *start = section;

    for (size_t i = 0; i < parts; i++)
    {
        const char *end = strpbrk(start, "-./");
        if (end == NULL)
        {
            end = start + strlen(start);
        }

        size_t len = end - start;

        vector_push(&result, ctu_strndup(start, len));
        start = end + 1;
    }

    char *tail = ctu_strdup(start);
    vector_push(&result, tail);

    return result;
}

// only the first segment of a namespace starts with `N`
static const char *get_mangle_prefix(size_t i)
{
    if (i == 0) { return "N"; }
    return "";
}

// TODO: we should just require frontends to pass us namespace segments properly
static char *mangle_section(size_t index, char *src, const char *part)
{
    char *dst = src;

    vector_t *sections = split_section(part);
    if (sections == NULL)
    {
        return format("%s%s%zu%s", dst, get_mangle_prefix(index), strlen(part), part);
    }
    
    char *head = vector_get(sections, 0);
    dst = format("%s%s%zu%s", dst, get_mangle_prefix(index), strlen(head), head);

    for (size_t j = 1; j < vector_len(sections); j++)
    {
        const char *section = vector_get(sections, j);
        dst = format("%s%zu%s", dst, strlen(section), section);
    }

    return dst;
}

static const char *flow_make_name(ssa_t *ssa, const h2_t *symbol)
{
    const h2_attrib_t *attribs = h2_get_attrib(symbol);
    if (attribs->mangle != NULL)
    {
        return attribs->mangle;
    }

    const char *part = h2_get_name(symbol);

    size_t parts = vector_len(ssa->namePath);
    if (parts == 0)
    {
        return format("_Z%zu%s", strlen(part), part);
    }

    char *result = ctu_strdup("_Z");
    for (size_t i = 0; i < parts; i++)
    {
        const char *ns = vector_get(ssa->namePath, i);
        result = mangle_section(i, result, ns);
    }

    result = format("%s%zu%sE", result, strlen(part), part);

    if (!h2_is(symbol, eHlir2DeclFunction))
    { 
        return result;
    }

    size_t len = vector_len(symbol->params);
    for (size_t i = 0; i < len; i++)
    {
        const h2_t *param = vector_get(symbol->params, i);
        result = format("%s%s", result, mangle_arg(ssa, param));
    }

    return result;
}

static ssa_flow_t *flow_new(ssa_t *ssa, const h2_t *symbol, ssa_type_t *type)
{
    UNUSED(ssa);

    ssa_flow_t *flow = ctu_malloc(sizeof(ssa_flow_t));
    flow->name = flow_make_name(ssa, symbol);
    flow->type = type;

    const h2_attrib_t *attribs = h2_get_attrib(symbol);

    if (is_imported(attribs->link))
    {
        flow->symbol = attribs->mangle;
    }
    else
    {
        flow->entry = NULL;
    }

    // TODO: a little iffy
    flow->link = attribs->link;
    flow->visible = attribs->visible;

    return flow;
}

static ssa_flow_t *global_new(ssa_t *ssa, const h2_t *global)
{
    ssa_type_t *type = type_new(ssa, h2_get_type(global));
    ssa_flow_t *flow = flow_new(ssa, global, type);
    
    if (!is_imported(flow->link))
    {
        flow->value = NULL;
    }

    return flow;
}

static ssa_flow_t *function_new(ssa_t *ssa, const h2_t *function)
{
    ssa_type_t *type = type_new(ssa, h2_get_type(function));
    ssa_flow_t *flow = flow_new(ssa, function, type);

    if (!is_imported(flow->link))
    {
        flow->locals = vector_new(0);
    }

    return flow;
}

static void fwd_global(ssa_t *ssa, const h2_t *global)
{
    ssa_flow_t *flow = global_new(ssa, global);
    map_set_ptr(ssa->globals, global, flow);
}

static void fwd_function(ssa_t *ssa, const h2_t *function)
{
    ssa_flow_t *flow = function_new(ssa, function);
    map_set_ptr(ssa->functions, function, flow);
}

static ssa_operand_t compile_rvalue(ssa_t *ssa, const h2_t *hlir);
static ssa_operand_t compile_stmt(ssa_t *ssa, const h2_t *stmt);

static ssa_type_t *new_digit_type(ssa_t *ssa, digit_t width, sign_t sign, const char *name)
{
    ssa_type_result_t result = ssa_anytype_new(ssa, NULL, name, eTypeDigit);
    ssa_type_t *it = result.type;
    it->digit = width;
    it->sign = sign;
    return it;
}

static ssa_type_t *ssa_get_digit_type(ssa_t *ssa, const h2_t *digit)
{
    CTASSERTF(h2_is(digit, eHlir2ExprDigit), "expected digit literal, got %s", hlir_kind_to_string(h2_get_kind(digit)));

    const h2_t *type = h2_follow_type(digit);

    CTASSERT(h2_is(type, eHlir2TypeDigit));

    return new_digit_type(ssa, type->digit, type->sign, h2_get_name(type));
}

static ssa_type_t *ssa_get_decimal_type(ssa_t *ssa, const h2_t *decimal)
{
    CTASSERTF(h2_is(decimal, eHlir2ExprDecimal), "expected decimal literal, got %s", hlir_kind_to_string(h2_get_kind(decimal)));

    const h2_t *type = h2_follow_type(decimal);

    CTASSERT(h2_is(type, eHlir2TypeDecimal));

    return ssa_anytype_new(ssa, NULL, "float", eTypeDecimal).type;
}

static ssa_type_t *ssa_get_bool_type(ssa_t *ssa)
{
    return ssa_anytype_new(ssa, NULL, "bool", eTypeBool).type;
}

static ssa_type_t *ssa_get_string_type(ssa_t *ssa)
{
    return ssa_anytype_new(ssa, NULL, "string", eTypeString).type;
}

static ssa_value_t *value_digit_new(ssa_t *ssa, const h2_t *hlir)
{
    ssa_type_t *type = ssa_get_digit_type(ssa, hlir);
    ssa_value_t *it = ssa_value_digit_new(hlir->digitValue, type);
    return it;
}

static ssa_value_t *ssa_value_decimal_new(ssa_t *ssa, const h2_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_decimal_type(ssa, hlir), true);

    memcpy(it->decimal, hlir->decimal, sizeof(mpq_t));

    return it;
}

static ssa_value_t *value_bool_new(ssa_t *ssa, const h2_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_bool_type(ssa), true);
    it->boolean = hlir->boolValue;
    return it;
}

static ssa_value_t *value_string_new(ssa_t *ssa, const h2_t *hlir)
{
    ssa_value_t *it = ssa_value_new(ssa_get_string_type(ssa), true);
    it->stringValue = hlir->stringValue;
    it->stringLength = hlir->stringLength;
    return it;
}

static ssa_operand_t compile_digit(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_digit_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_decimal(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = ssa_value_decimal_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_string(ssa_t *ssa, const h2_t *hlir)
{
    // TODO: string
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_string_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_bool(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_bool_new(ssa, hlir)
    };

    return op;
}

static ssa_operand_t compile_binary(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t lhs = compile_rvalue(ssa, hlir->lhs);
    ssa_operand_t rhs = compile_rvalue(ssa, hlir->rhs);

    ssa_step_t step = {
        .opcode = eOpBinary,
        .type = type_new(ssa, h2_get_type(hlir)),
        .binary = {
            .op = hlir->binary,
            .lhs = lhs,
            .rhs = rhs
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_load(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t name = compile_rvalue(ssa, hlir->load);

    ssa_step_t step = {
        .opcode = eOpLoad,
        .type = type_new(ssa, h2_get_type(hlir->load)),
        .load = {
            .src = name
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_global_ref(ssa_t *ssa, const h2_t *hlir)
{
    const ssa_flow_t *ref = map_get_ptr(ssa->globals, hlir);

    ssa_operand_t op = {
        .kind = eOperandGlobal,
        .global = ref
    };

    return op;
}

static ssa_operand_t compile_local_ref(ssa_t *ssa, const h2_t *hlir)
{
    size_t index = (uintptr_t)map_get_ptr(ssa->currentLocals, hlir);

    ssa_operand_t op = {
        .kind = eOperandLocal,
        .local = index
    };

    return op;
}

static ssa_operand_t compile_cast(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t op = compile_rvalue(ssa, hlir->expr);
    const ssa_type_t *type = type_new(ssa, h2_get_type(hlir));

    ssa_step_t step = {
        .opcode = eOpCast,
        .type = type,
        .cast = {
            .op = hlir->cast,
            .operand = op,

            .type = type
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_call(ssa_t *ssa, const h2_t *hlir)
{
    size_t len = vector_len(hlir->args);
    ssa_operand_t *args = ctu_malloc(sizeof(ssa_operand_t) * MAX(len, 1));
    for (size_t i = 0; i < len; i++)
    {
        const h2_t *arg = vector_get(hlir->args, i);
        args[i] = compile_rvalue(ssa, arg);
    }

    ssa_operand_t func = compile_rvalue(ssa, hlir->callee);

    ssa_step_t step = {
        .opcode = eOpCall,
        .type = type_new(ssa, h2_get_type(hlir)),
        .call = {
            .args = args,
            .len = len,
            .symbol = func
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_name(ssa_t *ssa, const h2_t *hlir)
{
    const ssa_flow_t *ref = map_get_ptr(ssa->functions, hlir);
    CTASSERT(ref != NULL);

    ssa_operand_t op = {
        .kind = eOperandFunction,
        .function = ref
    };

    return op;
}

static ssa_operand_t compile_lvalue(ssa_t *ssa, const h2_t *hlir);

static ssa_operand_t compile_global_lvalue(ssa_t *ssa, const h2_t *hlir)
{
    const ssa_flow_t *ref = map_get_ptr(ssa->globals, hlir);
    CTASSERT(ref != NULL);

    ssa_operand_t op = {
        .kind = eOperandGlobal,
        .global = ref
    };

    return op;
}

static ssa_operand_t compile_unary(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t op = compile_rvalue(ssa, hlir->operand);

    ssa_step_t step = {
        .opcode = eOpUnary,
        .type = type_new(ssa, h2_get_type(hlir)),
        .unary = {
            .op = hlir->unary,
            .operand = op
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_local_lvalue(ssa_t *ssa, const h2_t *hlir)
{
    const size_t idx = (size_t)(uintptr_t)map_get_ptr(ssa->currentLocals, hlir);

    ssa_operand_t op = {
        .kind = eOperandLocal,
        .local = idx
    };

    return op;
}

static size_t get_offset(const h2_t *object, const h2_t *member)
{
    const h2_t *type = h2_get_type(object);
    if (h2_is(type, eHlirPointer))
    {
        type = type->ptr;
    }

    CTASSERTF(h2_is(type, eHlirStruct), "%s", hlir_kind_to_string(h2_get_kind(type)));

    size_t field = vector_find(type->fields, member);

    CTASSERT(field != SIZE_MAX);

    return field;
}

static ssa_operand_t compile_access(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t read = compile_lvalue(ssa, hlir->object);
    size_t offset = get_offset(hlir->object, hlir->member);

    ssa_step_t step = {
        .opcode = eOpOffset,
        .type = type_new(ssa, h2_get_type(hlir)),
        .offset = {
            .object = read,
            .field = offset
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t operand_bb(const ssa_block_t *dst)
{
    ssa_operand_t op = {
        .kind = eOperandBlock,
        .bb = dst
    };

    return op;
}

static ssa_operand_t operand_empty(void)
{
    ssa_operand_t op = {
        .kind = eOperandEmpty
    };

    return op;
}

static ssa_operand_t compile_param(ssa_t *ssa, const h2_t *hlir)
{
    const void *ref = map_get_ptr(ssa->currentParams, hlir);

    ssa_operand_t op = {
        .kind = eOperandParam,
        .param = (uintptr_t)ref
    };

    return op;
}

static ssa_operand_t compile_index_lvalue(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t read = compile_lvalue(ssa, hlir->array);
    ssa_operand_t index = compile_rvalue(ssa, hlir->index);

    ssa_step_t step = {
        .opcode = eOpIndex,
        .type = type_new(ssa, h2_get_type(hlir)),
        .index = {
            .array = read,
            .index = index
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_lvalue(ssa_t *ssa, const h2_t *hlir)
{
    h2_kind_t kind = h2_get_kind(hlir);
    switch (kind)
    {
    case eHlirGlobal:
        return compile_global_lvalue(ssa, hlir);

    case eHlirLocal:
        return compile_local_lvalue(ssa, hlir);

    case eHlirParam:
        return compile_param(ssa, hlir);

    case eHlirAccess:
        return compile_access(ssa, hlir);

    case eHlirIndex:
        return compile_index_lvalue(ssa, hlir);

    default:
        report(ssa->reports, eInternal, h2_get_node(hlir), "compile-lvalue %s", hlir_to_string(hlir));
        return operand_empty();
    }
}

static ssa_operand_t compile_addr(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t op = compile_lvalue(ssa, hlir->expr);

    ssa_step_t step = {
        .opcode = eOpAddr,
        .type = type_new(ssa, h2_get_type(hlir)),
        .addr = {
            .expr = op
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_sizeof(ssa_t *ssa, const h2_t *hlir)
{
    ssa_type_t *type = type_new(ssa, hlir->operand);

    ssa_step_t step = {
        .opcode = eOpSizeOf,
        .type = new_digit_type(ssa, eDigitSize, eUnsigned, "size"),
        .size = {
            .type = type
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_builtin(ssa_t *ssa, const h2_t *hlir)
{
    switch (hlir->builtin)
    {
    case eBuiltinSizeOf:
        return compile_sizeof(ssa, hlir);
    
    default:
        report(ssa->reports, eInternal, h2_get_node(hlir), "compile-builtin %d", hlir->builtin);
        return operand_empty();
    }
}

static ssa_operand_t compile_index(ssa_t *ssa, const h2_t *hlir)
{
    ssa_operand_t op = compile_rvalue(ssa, hlir->array);
    ssa_operand_t index = compile_rvalue(ssa, hlir->index);

    ssa_step_t step = {
        .opcode = eOpIndex,
        .type = type_new(ssa, h2_get_type(hlir)),
        .index = {
            .array = op,
            .index = index
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_rvalue(ssa_t *ssa, const h2_t *hlir)
{
    CTASSERTF(hlir != NULL, "compile-rvalue(hlir = NULL) { flow = %s, block = %s }", ssa->currentFlow->name, ssa->currentBlock->id);
    h2_kind_t kind = h2_get_kind(hlir);
    switch (kind)
    {
    case eHlirEnumCase:
        return compile_rvalue(ssa, hlir->value);

    case eHlir2ExprDigit:
        return compile_digit(ssa, hlir);

    case eHlir2ExprDecimal:
        return compile_decimal(ssa, hlir);

    case eHlir2ExprString:
        return compile_string(ssa, hlir);

    case eHlir2ExprBool:
        return compile_bool(ssa, hlir);

    case eHlir2ExprUnary:
        return compile_unary(ssa, hlir);

    case eHlir2ExprBinary:
        return compile_binary(ssa, hlir);

    case eHlir2ExprLoad:
        return compile_load(ssa, hlir);

    case eHlir2DeclGlobal:
        return compile_global_ref(ssa, hlir);

    case eHlir2DeclLocal:
        return compile_local_ref(ssa, hlir);

    case eHlir2ExprCast:
        return compile_cast(ssa, hlir);

    case eHlir2ExprCall:
        return compile_call(ssa, hlir);

    case eHlirFunction:
        return compile_name(ssa, hlir);

    case eHlir2DeclParam:
        return compile_param(ssa, hlir);

    case eHlirAccess:
        return compile_access(ssa, hlir);

    case eHlirAddr:
        return compile_addr(ssa, hlir);

    case eHlirBuiltin:
        return compile_builtin(ssa, hlir);

    case eHlirIndex:
        return compile_index(ssa, hlir);

    default:
        report(ssa->reports, eInternal, h2_get_node(hlir), "compile-rvalue %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static ssa_operand_t compile_stmts(ssa_t *ssa, const h2_t *stmt)
{
    for (size_t i = 0; i < vector_len(stmt->stmts); i++)
    {
        compile_stmt(ssa, vector_get(stmt->stmts, i));
    }

    return operand_bb(ssa->currentBlock);
}

static ssa_operand_t compile_assign(ssa_t *ssa, const h2_t *stmt)
{
    ssa_operand_t dst = compile_lvalue(ssa, stmt->dst);
    ssa_operand_t src = compile_rvalue(ssa, stmt->src);

    ssa_step_t step = {
        .opcode = eOpStore,
        .store = {
            .dst = dst,
            .src = src
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_compare_compare(ssa_t *ssa, const h2_t *cond)
{
    ssa_operand_t lhs = compile_rvalue(ssa, cond->lhs);
    ssa_operand_t rhs = compile_rvalue(ssa, cond->rhs);

    ssa_step_t step = {
        .opcode = eOpCompare,
        .type = ssa_get_bool_type(ssa),
        .compare = {
            .op = cond->compare,
            .lhs = lhs,
            .rhs = rhs
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_compare_bool(ssa_t *ssa, const h2_t *cond)
{
    ssa_operand_t op = {
        .kind = eOperandImm,
        .value = value_bool_new(ssa, cond)
    };

    return op;
}

static ssa_operand_t compile_compare(ssa_t *ssa, const h2_t *cond)
{
    h2_kind_t kind = h2_get_kind(cond);
    switch (kind)
    {
    case eHlir2ExprCompare:
        return compile_compare_compare(ssa, cond);

    case eHlir2ExprBool:
        return compile_compare_bool(ssa, cond);

    default:
        report(ssa->reports, eInternal, h2_get_node(cond), "compile-compare %s", hlir_kind_to_string(kind));
        return operand_empty();
    }
}

static ssa_operand_t compile_loop(ssa_t *ssa, const h2_t *stmt)
{
    ssa_block_t *loop = block_gen(ssa, "loop");
    ssa_block_t *tail = block_gen(ssa, "tail");

    ssa_step_t step = {
        .opcode = eOpJmp,
        .jmp = {
            .label = operand_bb(loop)
        }
    };

    add_step(ssa, step);

    ssa->currentBlock = loop;
    compile_stmt(ssa, stmt->then);

    ssa_step_t ret = {
        .opcode = eOpBranch,
        .branch = {
            .cond = compile_compare(ssa, stmt->cond),
            .truthy = operand_bb(loop),
            .falsey = operand_bb(tail)
        }
    };

    add_step(ssa, ret);

    ssa->currentBlock = tail;

    return operand_bb(tail);
}

static ssa_operand_t compile_branch(ssa_t *ssa, const h2_t *stmt)
{
    ssa_block_t *then = block_gen(ssa, "then");
    ssa_block_t *other = stmt->other != NULL ? block_gen(ssa, "other") : NULL;
    ssa_block_t *tail = block_gen(ssa, "tail");

    ssa_step_t ret = {
        .opcode = eOpJmp,
        .jmp = {
            .label = operand_bb(tail)
        }
    };

    ssa_step_t step = {
        .opcode = eOpBranch,
        .branch = {
            .cond = compile_compare(ssa, stmt->cond),
            .truthy = operand_bb(then),
            .falsey = other != NULL ? operand_bb(other) : operand_bb(tail)
        }
    };

    add_step(ssa, step);

    // if true
    ssa->currentBlock = then;
    compile_stmt(ssa, stmt->then);
    add_step(ssa, ret);

    // if false
    if (other != NULL) 
    {
        ssa->currentBlock = other;
        compile_stmt(ssa, stmt->other);
        add_step(ssa, ret);
    }

    ssa->currentBlock = tail;

    return operand_bb(tail);
}

static ssa_operand_t compile_return(ssa_t *ssa, const h2_t *stmt)
{
    const h2_t *result = stmt->result;
    ssa_operand_t op = result != NULL ? compile_rvalue(ssa, result) : operand_empty();

    ssa_step_t step = {
        .opcode = eOpReturn,
        .ret = {
            .value = op,
        }
    };

    return add_step(ssa, step);
}

static ssa_operand_t compile_stmt(ssa_t *ssa, const h2_t *stmt)
{
    h2_kind_t kind = h2_get_kind(stmt);
    switch (kind)
    {
    case eHlir2StmtBlock:
        return compile_stmts(ssa, stmt);

    case eHlir2ExprCall:
        return compile_call(ssa, stmt);

    case eHlir2StmtAssign:
        return compile_assign(ssa, stmt);

    case eHlir2StmtLoop:
        return compile_loop(ssa, stmt);

    case eHlir2StmtBranch:
        return compile_branch(ssa, stmt);

    case eHlir2ExprLoad:
        return compile_rvalue(ssa, stmt);

    case eHlir2StmtReturn:
        return compile_return(ssa, stmt);

    default:
        report(ssa->reports, eInternal, h2_get_node(stmt), "compile-stmt %s", hlir_to_string(stmt));
        return operand_empty();
    }
}

static void compile_flow(ssa_t *ssa, ssa_flow_t *flow)
{
    ssa_block_t *block = block_new("entry");
    flow->entry = block;

    ssa->currentBlock = block;
    ssa->currentFlow = flow;
    ssa->stepIdx = 0;
    ssa->blockIdx = 0;
}

static void compile_global(ssa_t *ssa, const h2_t *global)
{
    ssa_flow_t *flow = map_get_ptr(ssa->globals, global);

    if (is_imported(flow->linkage))
    {
        return;
    }

    compile_flow(ssa, flow);

    ssa_operand_t result = global->value == NULL ? operand_empty() : compile_rvalue(ssa, global->value);
    ssa_step_t step = {
        .opcode = eOpReturn,
        .ret = {
            .value = result
        }
    };
    add_step(ssa, step);
}

static void compile_function(ssa_t *ssa, const h2_t *function)
{
    ssa_flow_t *flow = map_get_ptr(ssa->functions, function);
    CTASSERT(flow != NULL);

    if (is_imported(flow->linkage))
    {
        return;
    }

    size_t totalLocals = vector_len(function->locals);
    size_t totalParams = vector_len(function->params);

    ssa->currentLocals = map_optimal(totalLocals);
    ssa->currentParams = map_optimal(totalParams);

    flow->locals = vector_of(totalLocals);
    
    for (size_t i = 0; i < totalLocals; i++) 
    {
        const h2_t *local = vector_get(function->locals, i);
        map_set_ptr(ssa->currentLocals, local, (void*)(uintptr_t)i);
        vector_set(flow->locals, i, type_new(ssa, h2_get_type(local)));
    }

    for (size_t i = 0; i < totalParams; i++)
    {
        const h2_t *param = vector_get(function->params, i);
        map_set_ptr(ssa->currentParams, param, (void*)(uintptr_t)i);
    }

    compile_flow(ssa, flow);

    // TODO: idk if this is right
    if (function->body == NULL) 
    { 
        map_set(ssa->importedSymbols, function->name, flow);
        return;
    }

    compile_stmt(ssa, function->body);
}

static bool has_name(const char *id)
{
    return id == NULL || str_equal(id, "");
}

static void fwd_module(ssa_t *ssa, h2_t *mod)
{
    CTASSERT(h2_is(mod, eHlirModule));

    const char *name = mod->name;

    if (has_name(name))
    {
        vector_push(&ssa->namePath, (void*)mod->name);
    }

    for (size_t j = 0; j < vector_len(mod->globals); j++)
    {
        fwd_global(ssa, vector_get(mod->globals, j));
    }

    for (size_t j = 0; j < vector_len(mod->functions); j++)
    {
        fwd_function(ssa, vector_get(mod->functions, j));
    }

    if (has_name(name))
    {
        vector_drop(ssa->namePath);
    }
}

ssa_module_t *ssa_gen_module(reports_t *reports, vector_t *mods)
{
    ssa_t ssa = { 
        .reports = reports,
        .aggregates = map_optimal(128),
        .globals = map_optimal(128),
        .functions = map_optimal(128),
        .strings = set_new(128),
        .importedSymbols = map_optimal(128),
        .namePath = vector_new(16),
        .emptyType = ssa_type_empty_new("empty"),
        .stubType = ssa_get_bool_type(&ssa)
    };

    size_t len = vector_len(mods);
    for (size_t i = 0; i < len; i++)
    {
        fwd_module(&ssa, vector_get(mods, i));
    }    

    for (size_t i = 0; i < len; i++)
    {
        h2_t *mod = vector_get(mods, i);
        for (size_t j = 0; j < vector_len(mod->globals); j++)
        {
            compile_global(&ssa, vector_get(mod->globals, j));
        }

        for (size_t j = 0; j < vector_len(mod->functions); j++)
        {
            compile_function(&ssa, vector_get(mod->functions, j));
        }
    }

    section_t symbols = {
        .types = map_values(ssa.aggregates),
        .globals = map_values(ssa.globals),
        .functions = map_values(ssa.functions)
    };

    ssa_module_t *mod = ctu_malloc(sizeof(ssa_module_t));
    mod->symbols = symbols;

    return mod;
}