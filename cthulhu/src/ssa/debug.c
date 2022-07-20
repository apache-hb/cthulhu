#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/query.h"

#include "base/macros.h"

#include "std/stream.h"
#include "std/str.h"
#include "std/vector.h"
#include "report/report.h"

typedef struct 
{
    reports_t *reports;
    module_t *mod;

    stream_t *out;
} ssa_debug_t;

static char *format_digit(int_t digitKind)
{
    const char *width = hlir_digit_to_string(digitKind.width);
    const char *sign = hlir_sign_to_string(digitKind.sign);

    return format("digit(%s:%s)", width, sign);
}

static const char *format_type(ssa_debug_t *dbg, const type_t *type)
{
    switch (type->kind)
    {
    case eLiteralBool: return "bool";
    case eLiteralVoid: return "void";
    case eLiteralString: return "str";
    case eLiteralInt:
        return format_digit(type->digitKind);
    case eLiteralEmpty: return "";

    default:
        ctu_assert(dbg->reports, "format-type %d", type->kind);
        return "";
    }
}

static const char *format_types(ssa_debug_t *dbg, vector_t *types)
{
    size_t len = vector_len(types);
    if (len == 0)
    {
        return "";
    }

    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const char *name = format_type(dbg, vector_get(types, i));
        vector_set(result, i, format("%zu = %s", i, name));
    }

    return str_join(", ", result);
}

static const char *emit_value(ssa_debug_t *dbg, value_t value)
{
    switch (value.type.kind)
    {
    case eLiteralBool:
        return value.boolean ? "$true" : "$false";

    case eLiteralInt:
        return format("$%s", mpz_get_str(NULL, 10, value.digit));

    case eLiteralString:
        return format("\"%s\"", str_normalizen(value.string, value.length));

    case eLiteralEmpty:
        return "";

    case eLiteralVoid:
    default:
        ctu_assert(dbg->reports, "emit-value %d", value.type.kind);
        return "";
    }
}

static const char *emit_operand(ssa_debug_t *dbg, operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandArg:
        return format("param[%zu]", operand.param);
    case eOperandReg:
        return format("%%%zu", operand.reg);
    case eOperandLocal:
        return format("local[%zu]", operand.local);
    case eOperandRef:
        return format("<%s>", operand.ref->name);
    case eOperandEmpty:
        return "";
    case eOperandImm:
        return emit_value(dbg, operand.value);

    default:
        ctu_assert(dbg->reports, "emit-operand %d", operand.kind);
        return "";
    }
}

#define STREAM_WRITEF(stream, ...) stream_write(stream, format(__VA_ARGS__))

static void write_call(ssa_debug_t *dbg, size_t idx, step_t step)
{
    vector_t *args = vector_of(step.count);
    for (size_t i = 0; i < step.count; i++)
    {
        const char *arg = emit_operand(dbg, step.args[i]);
        vector_set(args, i, (char*)arg);
    }

    const char *joined = str_join(", ", args);

    STREAM_WRITEF(dbg->out, "    %%%zu = call %s (%s)\n", idx, emit_operand(dbg, step.call), joined);
}

static void write_step(ssa_debug_t *dbg, size_t idx, step_t step)
{
    switch (step.op)
    {
    case eOpReturn:
        STREAM_WRITEF(dbg->out, "    ret %s\n", emit_operand(dbg, step.result));
        break;

    case eOpLoad:
        STREAM_WRITEF(dbg->out, "    %%%zu = load %s\n", idx, emit_operand(dbg, step.src));
        break;

    case eOpValue:
        STREAM_WRITEF(dbg->out, "    %%%zu = %s\n", idx, emit_operand(dbg, step.operand));
        break;

    case eOpCall:
        write_call(dbg, idx, step);
        break;

    case eOpBinary:
        STREAM_WRITEF(dbg->out, "    %%%zu = binary(%s) %s %s\n", idx, binary_name(step.binary), emit_operand(dbg, step.lhs), emit_operand(dbg, step.rhs));
        break;

    case eOpStore:
        STREAM_WRITEF(dbg->out, "    store %s = %s\n", emit_operand(dbg, step.dst), emit_operand(dbg, step.src));
        break;

    default:
        ctu_assert(dbg->reports, "write-step %d", step.op);
        break;
    }
}

static void write_flow(ssa_debug_t *dbg, const flow_t *flow)
{
    stream_write(dbg->out, format("def %s: %s\n", flow->name, format_type(dbg, &flow->result)));
    
    stream_write(dbg->out, format("  params: [%s]\n", format_types(dbg, flow->params)));

    if (flow->locals != NULL)
    {
        stream_write(dbg->out, format("  locals: [%s]\n", format_types(dbg, flow->locals)));
    }

    if (flow->steps == NULL)
    {
        return;
    }

    for (size_t i = 0; i < flow->stepsLen; i++) 
    {
        write_step(dbg, i, flow->steps[i]);
    }
}

static void write_section(ssa_debug_t *dbg, vector_t *flows, const char *section)
{
    size_t len = vector_len(flows);

    if (len == 0)
    {
        return;
    }

    stream_write(dbg->out, format("section: %s\n", section));

    for (size_t i = 0; i < len; i++)
    {
        write_flow(dbg, vector_get(flows, i));
    }
}

stream_t *ssa_debug(reports_t *reports, module_t *mod)
{
    ssa_debug_t dbg = {
        .reports = reports,
        .mod = mod,

        .out = stream_new(0x1000)
    };

    if (mod->cliEntry != NULL)
    {
        stream_write(dbg.out, format("#cli-entry := %s\n", mod->cliEntry->name));
    }

    if (mod->guiEntry != NULL)
    {
        stream_write(dbg.out, format("#gui-entry := %s\n", mod->guiEntry->name));
    }

    write_section(&dbg, mod->imports, "imports");
    write_section(&dbg, mod->exports, "exports");
    write_section(&dbg, mod->internals, "internals");

    return dbg.out;
}
