#include "cthulhu/ssa/ssa.h"

#include "cthulhu/hlir/query.h"

#include "base/macros.h"

#include "std/stream.h"
#include "std/str.h"
#include "std/vector.h"

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

static const char *format_type(type_t *type)
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
        UNREACHABLE();
    }
}

static const char *format_locals(vector_t *locals)
{
    size_t len = vector_len(locals);
    if (len == 0)
    {
        return "";
    }

    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const char *name = format_type(vector_get(locals, i));
        vector_set(result, i, format("%zu = %s", i, name));
    }

    return str_join(", ", result);
}

static void write_step(ssa_debug_t *dbg, size_t idx, step_t step)
{
    UNUSED(dbg);
    UNUSED(idx);
    UNUSED(step);
}

static void write_flow(ssa_debug_t *dbg, const flow_t *flow)
{
    stream_write(dbg->out, format("def %s:\n", flow->name));
    
    stream_write(dbg->out, format("  locals: [%s]\n", format_locals(flow->locals)));

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
