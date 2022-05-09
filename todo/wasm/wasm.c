#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/util/map.h"

#include <string.h>

/**
 * this file is based off the <a href="https://webassembly.github.io/spec/core/binary/index.html">wasm binary format
 * specification</a>
 */

// https://webassembly.github.io/spec/core/binary/modules.html#sections
typedef enum
{
    WASM_CUSTOM_SECTION = 0,
    WASM_TYPE_SECTION = 1,
    WASM_IMPORT_SECTION = 2,
    WASM_FUNCTION_SECTION = 3,
    WASM_TABLE_SECTION = 4,
    WASM_MEMORY_SECTION = 5,
    WASM_GLOBAL_SECTION = 6,
    WASM_EXPORT_SECTION = 7,
    WASM_START_SECTION = 8,
    WASM_ELEMENT_SECTION = 9,
    WASM_CODE_SECTION = 10,
    WASM_DATA_SECTION = 11,
    WASM_DATA_COUNT_SECTION = 12,
    WASM_SECTION_TOTAL
} wasm_section_t;

// https://webassembly.github.io/spec/core/binary/modules.html#binary-magic
static const uint8_t kWasmMagic[] = {'\0', 'a', 's', 'm'};

// https://webassembly.github.io/spec/core/binary/modules.html#binary-version
static const uint8_t kWasmVersion[] = {1, 0, 0, 0};

#define FOR_EACH_MODULE(mods, name, ...)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        for (size_t i = 0; i < vector_len(mods); i++)                                                                  \
        {                                                                                                              \
            hlir_t *name = vector_get(mods, i);                                                                        \
            __VA_ARGS__;                                                                                               \
        }                                                                                                              \
    } while (0)

#define FOR_EACH_ITEM_IN_MODULES(item, name, mods, ...)                                                                \
    FOR_EACH_MODULE(mods, mod, {                                                                                       \
        for (size_t j = 0; j < vector_len(mod->item); j++)                                                             \
        {                                                                                                              \
            hlir_t *name = vector_get(mod->item, j);                                                                   \
            __VA_ARGS__;                                                                                               \
        }                                                                                                              \
    })

typedef struct
{
    reports_t *reports;
    file_t file;
    wasm_settings_t settings;

    map_t *symbolIndices;

    uint32_t totalGlobals;
    uint32_t totalExports;
    uint32_t totalImports;

    size_t totalDataSegments;
    size_t totalTypes;

    stream_t *sections[WASM_SECTION_TOTAL];
} wasm_t;

typedef struct
{
    wasm_t *wasm;

    stream_t *stream;
} wasm_stream_t;

#define WASM_CONST 0x00
#define WASM_MUT 0x01

#define WASM_NUMTYPE_I32 0x7F
#define WASM_NUMTYPE_I64 0x7E

#define WASM_REFTYPE_FUNCREF 0x70
#define WASM_REFTYPE_EXTERNREF 0x6F

/* https://webassembly.github.io/spec/core/binary/instructions.html#numeric-instructions */
#define WASM_INST_I32_CONST 0x41
#define WASM_INST_I64_CONST 0x42

#define WASM_INST_I32_ADD 0x6A
#define WASM_INST_I64_ADD 0x7C

/* https://webassembly.github.io/spec/core/binary/instructions.html#variable-instructions */
#define WASM_INST_LOCAL_GET 0x20
#define WASM_INST_LOCAL_SET 0x21

#define WASM_INST_GLOBAL_GET 0x23
#define WASM_INST_GLOBAL_SET 0x24

/* https://webassembly.github.io/spec/core/binary/instructions.html#control-instructions */

#define WASM_BLOCKTYPE_EMPTY 0x40

#define WASM_INST_BLOCK 0x02
#define WASM_INST_CALL 0x10

#define WASM_INST_END 0x0B

#define WASM_EXPORTDESC_FUNC 0x00
#define WASM_EXPORTDESC_TABLE 0x01
#define WASM_EXPORTDESC_MEMORY 0x02
#define WASM_EXPORTDESC_GLOBAL 0x03

typedef struct
{
    uint8_t buffer[16]; // TODO: figure out max buffer size based on sizeof(uintmax_t)
    size_t length;
} leb128_t;

static leb128_t ui_leb128(uint64_t value)
{
    leb128_t result = {0};

    bool more = true;
    do
    {
        uint8_t c = value & 0x7F;
        value >>= 7;
        more = value != 0;

        result.buffer[result.length++] = c | (more ? 0x80 : 0);
    } while (more);

    return result;
}

static leb128_t si_leb128(int64_t value)
{
    leb128_t result = {0};

    bool more = true;
    do
    {
        uint8_t c = value & 0x7F;
        value >>= 7;
        more = c & 0x40 ? value != -1 : value != 0;

        result.buffer[result.length++] = c | (more ? 0x80 : 0);
    } while (more);

    return result;
}

static wasm_stream_t wasm_stream(wasm_t *wasm, stream_t *stream)
{
    wasm_stream_t result = {.wasm = wasm, .stream = stream};
    return result;
}

static wasm_stream_t wasm_stream_from_section(wasm_t *wasm, wasm_section_t section)
{
    return wasm_stream(wasm, wasm->sections[section]);
}

static void wasm_set_symbol_index(wasm_t *wasm, const hlir_t *symbol, uintptr_t index)
{
    map_set_ptr(wasm->symbolIndices, symbol, (void *)index);
}

static uintptr_t wasm_get_symbol_index(wasm_t *wasm, const hlir_t *symbol)
{
    return (uintptr_t)map_get_ptr(wasm->symbolIndices, symbol);
}

static void wasm_stream_write(wasm_stream_t *stream, const void *data, size_t size)
{
    stream_write_bytes(stream->stream, data, size);
}

static void wasm_write_op(wasm_stream_t *stream, uint8_t op)
{
    wasm_stream_write(stream, &op, sizeof(op));
}

static void wasm_write_unsigned(wasm_stream_t *stream, uint64_t value)
{
    leb128_t leb = ui_leb128(value);
    wasm_stream_write(stream, leb.buffer, leb.length);
}

static void wasm_write_signed(wasm_stream_t *stream, uint64_t value)
{
    leb128_t leb = si_leb128(value);
    wasm_stream_write(stream, leb.buffer, leb.length);
}

static void wasm_write_symbol_index(wasm_stream_t *stream, const hlir_t *hlir)
{
    wasm_write_unsigned(stream, wasm_get_symbol_index(stream->wasm, hlir));
}

static void wasm_write_name(wasm_stream_t *stream, const char *name)
{
    size_t len = strlen(name);
    wasm_write_unsigned(stream, len);
    wasm_stream_write(stream, name, len);
}

static const char *wasm_mangle_name(const hlir_t *hlir)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    if (attribs->mangle != NULL)
    {
        return attribs->mangle;
    }

    const char *result = get_hlir_name(hlir);
    const hlir_t *parent = get_hlir_parent(hlir);

    while (parent != NULL)
    {
        const char *parentName = get_hlir_name(parent);
        result = format("%s::%s", parentName, result);
        parent = get_hlir_parent(parent);
    }

    return result;
}

static uint8_t wasm_get_exportdesc(wasm_t *wasm, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);

    switch (kind)
    {
    case HLIR_GLOBAL:
        return WASM_EXPORTDESC_GLOBAL;

    case HLIR_FUNCTION:
        return WASM_EXPORTDESC_FUNC;

    default:
        report(wasm->reports, INTERNAL, get_hlir_node(hlir), "unsupported export type %s", hlir_kind_to_string(kind));
        return 0xFF;
    }
}

static void wasm_write_export(wasm_t *wasm, const hlir_t *symbol)
{
    
    uint8_t exportDesc = wasm_get_exportdesc(wasm, symbol);
    const char *name = wasm_mangle_name(symbol);

    wasm_stream_t stream = wasm_stream_from_section(wasm, WASM_EXPORT_SECTION);

    wasm_write_name(&stream, name);
    wasm_write_op(&stream, exportDesc);
    wasm_write_symbol_index(&stream, symbol);

    wasm->totalExports += 1;
}

static void wasm_write_import(wasm_t *wasm, const hlir_t *symbol)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(symbol);
    uint8_t importDesc = wasm_get_exportdesc(wasm, symbol);
    const char *name = wasm_mangle_name(symbol);
    const char *module = attribs->module;

    wasm_stream_t stream = wasm_stream_from_section(wasm, WASM_IMPORT_SECTION);

    wasm_write_name(&stream, module != NULL ? module : wasm->settings.defaultModule);
    wasm_write_name(&stream, name);
    wasm_write_op(&stream, importDesc);
    wasm_write_symbol_index(&stream, symbol);

    wasm->totalImports += 1;
}

// https://webassembly.github.io/spec/core/binary/types.html#binary-mut
static uint8_t wasm_get_mut(wasm_t *wasm, const hlir_t *hlir, hlir_tags_t tags)
{
    if (tags & (TAG_ATOMIC | TAG_VOLATILE))
    {
        report(wasm->reports, ERROR, get_hlir_node(hlir), "atomic and volatile globals are not supported");
        return 0xFF;
    }

    return (tags & TAG_CONST) ? WASM_CONST : WASM_MUT;
}

// https://webassembly.github.io/spec/core/binary/types.html#binary-numtype
static uint8_t wasm_get_numtype(wasm_t *wasm, const hlir_t *hlir)
{
    digit_t digit = hlir->width;

    switch (digit)
    {
    case DIGIT_CHAR:
    case DIGIT_SHORT:
    case DIGIT_INT:
    case DIGIT_PTR:
    case DIGIT_SIZE: // TODO: is size_t a 32-bit integer?
        return WASM_NUMTYPE_I32;

    case DIGIT_LONG:
    case DIGIT_MAX:
        return WASM_NUMTYPE_I64;

    default:
        ctu_assert(wasm->reports, "unsupported digit %s", hlir_digit_to_string(digit));
        return 0xFF;
    }
}

// https://webassembly.github.io/spec/core/binary/types.html#binary-valtype
static uint8_t wasm_get_valtype(wasm_t *wasm, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case HLIR_DIGIT:
        return wasm_get_numtype(wasm, hlir);

    case HLIR_CLOSURE:
        return WASM_REFTYPE_FUNCREF;

    case HLIR_STRING:
    case HLIR_POINTER:
        return WASM_NUMTYPE_I32; // TODO: this will change if we target 64 bit as well

    default:
        report(wasm->reports, ERROR, get_hlir_node(hlir), "unsupported type %s", hlir_kind_to_string(kind));
        return 0xFF;
    }
}

static void wasm_emit_expr(wasm_stream_t *stream, const hlir_t *hlir);

static void wasm_emit_digit_literal(wasm_stream_t *stream, const hlir_t *hlir)
{
    const hlir_t *type = get_hlir_type(hlir);
    uint8_t kind = wasm_get_numtype(stream->wasm, type);

    if (kind == 0xFF)
    {
        return;
    }

    wasm_write_op(stream, kind == WASM_NUMTYPE_I32 ? WASM_INST_I32_CONST : WASM_INST_I64_CONST);

    if (type->sign == SIGN_UNSIGNED)
    {
        wasm_write_unsigned(stream, mpz_get_ui(hlir->digit));
    }
    else
    {
        wasm_write_signed(stream, mpz_get_si(hlir->digit));
    }
}

static void wasm_emit_binary(wasm_stream_t *stream, const hlir_t *hlir)
{
    const hlir_t *type = get_hlir_type(hlir);
    uint8_t kind = wasm_get_numtype(stream->wasm, type);

    if (kind == 0xFF)
    {
        return;
    }

    wasm_emit_expr(stream, hlir->lhs);
    wasm_emit_expr(stream, hlir->rhs);

    wasm_write_op(stream, kind == WASM_NUMTYPE_I32 ? WASM_INST_I32_ADD : WASM_INST_I64_ADD);
}

static void wasm_emit_stmts(wasm_stream_t *stream, const hlir_t *hlir)
{
    wasm_write_op(stream, WASM_INST_BLOCK);
    wasm_write_op(stream, WASM_BLOCKTYPE_EMPTY);

    for (size_t i = 0; i < vector_len(hlir->stmts); i++)
    {
        wasm_emit_expr(stream, vector_get(hlir->stmts, i));
    }

    wasm_write_op(stream, WASM_INST_END);
}

static void wasm_emit_assign(wasm_stream_t *stream, const hlir_t *hlir)
{
    wasm_emit_expr(stream, hlir->src);
    
    const hlir_t *dst = hlir->dst;

    if (hlir_is(dst, HLIR_LOCAL))
    {
        wasm_write_op(stream, WASM_INST_LOCAL_SET);
        wasm_write_unsigned(stream, dst->index);
    }
    else
    {
        wasm_write_op(stream, WASM_INST_GLOBAL_SET);
        wasm_write_symbol_index(stream, dst);
    }
}

static void wasm_emit_name(wasm_stream_t *stream, const hlir_t *hlir)
{
    const hlir_t *src = hlir->read;

    hlir_kind_t kind = get_hlir_kind(src);
    switch (kind)
    {
    case HLIR_LOCAL:
        wasm_write_op(stream, WASM_INST_LOCAL_GET);
        wasm_write_unsigned(stream, src->index);
        break;

    case HLIR_GLOBAL:
        wasm_write_op(stream, WASM_INST_GLOBAL_GET);
        wasm_write_symbol_index(stream, src);
        break;

    default:
        report(stream->wasm->reports, INTERNAL, get_hlir_node(src), "unsupported read type %s", hlir_kind_to_string(kind));
        break;
    }
}

static void wasm_emit_args(wasm_stream_t *stream, vector_t *args)
{
    for (size_t i = 0; i < vector_len(args); i++)
    {
        wasm_emit_expr(stream, vector_get(args, i));
    }
}

static void wasm_emit_call(wasm_stream_t *stream, const hlir_t *hlir)
{
    wasm_emit_args(stream, hlir->args);
    wasm_write_op(stream, WASM_INST_CALL);
    wasm_write_symbol_index(stream, hlir->call); // TODO: handle indirect calls
}

static void wasm_emit_expr(wasm_stream_t *stream, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case HLIR_DIGIT_LITERAL:
        wasm_emit_digit_literal(stream, hlir);
        break;

    case HLIR_BINARY:
        wasm_emit_binary(stream, hlir);
        break;

    case HLIR_STMTS:
        wasm_emit_stmts(stream, hlir);
        break;

    case HLIR_ASSIGN:
        wasm_emit_assign(stream, hlir);
        break;

    case HLIR_NAME:
        wasm_emit_name(stream, hlir);
        break;

    case HLIR_CALL:
        wasm_emit_call(stream, hlir);
        break;

    default:
        report(stream->wasm->reports, INTERNAL, get_hlir_node(hlir), "unsupported expression %s",
               hlir_kind_to_string(kind));
        break;
    }
}

/**
 * @brief check the source of a symbol and emit the appropriate import or export data
 * 
 * @return true if the symbol should be emitted
 */
static bool wasm_check_locality(wasm_t *wasm, const hlir_t *hlir)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    switch (attribs->linkage)
    {
    case LINK_EXPORTED:
        wasm_write_export(wasm, hlir);
        return true;

    case LINK_IMPORTED:
        wasm_write_import(wasm, hlir);
        return false;

    default:
        return true;
    }
}

/* https://webassembly.github.io/spec/core/binary/types.html#binary-globaltype */
static void wasm_emit_global_type(wasm_stream_t *stream, const hlir_t *hlir)
{
    if (!wasm_check_locality(stream->wasm, hlir))
    {
        return;
    }
    
    const hlir_t *type = get_hlir_type(hlir);
    const hlir_attributes_t *typeAttribs = get_hlir_attributes(type);
    uint8_t mut = wasm_get_mut(stream->wasm, hlir, typeAttribs->tags);
    uint8_t valtype = wasm_get_valtype(stream->wasm, type);

    uint8_t vals[] = {valtype, mut};

    wasm_stream_write(stream, vals, sizeof(vals));
}

/* https://webassembly.github.io/spec/core/binary/types.html#binary-functype */
static void wasm_emit_function_type(wasm_stream_t *stream, const hlir_t *hlir)
{   
    vector_t *params = closure_params(hlir);
    const hlir_t *result = closure_result(hlir);

    size_t totalParams = vector_len(params);

    wasm_write_unsigned(stream, totalParams);
    for (size_t i = 0; i < totalParams; i++)
    {
        const hlir_t *param = vector_get(params, i);
        uint8_t valtype = wasm_get_valtype(stream->wasm, param);
        wasm_write_op(stream, valtype);
    }
    
    if (!hlir_is(result, HLIR_VOID))
    {
        wasm_write_unsigned(stream, 1);
        uint8_t valtype = wasm_get_valtype(stream->wasm, result);
        wasm_write_op(stream, valtype);
    }
    else
    {
        wasm_write_unsigned(stream, 0);
    }
}

static void wasm_emit_global(wasm_t *wasm, const hlir_t *hlir)
{
    if (!wasm_check_locality(wasm, hlir))
    {
        return;
    }

    wasm_stream_t stream = wasm_stream_from_section(wasm, WASM_GLOBAL_SECTION);
    wasm_emit_global_type(&stream, hlir);

    if (!hlir_is(hlir->value, HLIR_DIGIT_LITERAL))
    {
        report(wasm->reports, INTERNAL, get_hlir_node(hlir), "global value must be a digit literal");
    }

    wasm_emit_expr(&stream, hlir->value);

    wasm_write_op(&stream, WASM_INST_END);

    wasm->totalGlobals += 1;
}

static void wasm_write_locals(wasm_stream_t *stream, vector_t *locals)
{
    size_t totalLocals = vector_len(locals);
    wasm_write_unsigned(stream, totalLocals);

    for (size_t i = 0; i < totalLocals; i++)
    {
        const hlir_t *local = vector_get(locals, i);
        const hlir_t *type = get_hlir_type(local);

        wasm_write_unsigned(stream, 1);
        uint8_t valtype = wasm_get_valtype(stream->wasm, type);
        wasm_write_op(stream, valtype);
    }
}

static void wasm_emit_function(wasm_t *wasm, const hlir_t *hlir)
{
    if (!wasm_check_locality(wasm, hlir))
    {
        return;
    }

    wasm_stream_t typeStream = wasm_stream_from_section(wasm, WASM_TYPE_SECTION);
    wasm_stream_t funcStream = wasm_stream_from_section(wasm, WASM_FUNCTION_SECTION);

    /* write out the function type */
    wasm_emit_function_type(&typeStream, hlir);

    /* then write its index to the type stream */
    wasm_write_unsigned(&funcStream, wasm->totalTypes);

    /* now write out the locals */

    stream_t *body = stream_new(0x100);
    wasm_stream_t bodyStream = wasm_stream(wasm, body);

    wasm_write_locals(&bodyStream, hlir->locals);
    wasm_emit_expr(&bodyStream, hlir->body);

    wasm->totalTypes += 1;
}

static void wasm_write_section(wasm_t *wasm, wasm_section_t section, uint32_t entries, file_t file)
{
    stream_t *stream = wasm->sections[section];
    uint32_t size = (uint32_t)stream_len(stream);
    uint8_t sec = (uint8_t)section;

    logverbose("writing section %d (%u bytes)", section, size);

    error_t error = 0;

    leb128_t actualEntries = ui_leb128(entries);
    leb128_t actualLength = ui_leb128(size + actualEntries.length);

    file_write(file, &sec, sizeof(sec), &error);                          // N:byte section id
    file_write(file, actualLength.buffer, actualLength.length, &error);   // size:u32 contents size
    file_write(file, actualEntries.buffer, actualEntries.length, &error); // n:u32 number of entries
    file_write(file, stream_data(stream), size, &error);                  // x:B bytes of data
}

void wasm_emit_modules(reports_t *reports, vector_t *modules, file_t output, wasm_settings_t settings)
{
    // first we collect the total number of symbols we have to we can make an
    // optimal symbol table size for better performance on larger programs
    size_t totalSymbols = 0;
    FOR_EACH_MODULE(modules, mod, { totalSymbols += vector_len(mod->globals); });
    FOR_EACH_MODULE(modules, mod, { totalSymbols += vector_len(mod->functions); });

    // then we create our wasm context
    wasm_t wasm = {
        .reports = reports,
        .file = output,
        .settings = settings,

        // use the size generated earlier to make a map
        .symbolIndices = map_optimal(totalSymbols),
    };

    for (int i = 0; i < WASM_SECTION_TOTAL; i++)
    {
        wasm.sections[i] = stream_new(0x1000);
    }

    //
    // wasm requires the index of a function to be known
    // before we can emit a call to it. so here we go over
    // every function and global to get their indices
    //

    uintptr_t totalGlobals = 0;
    uintptr_t totalFunctions = 0;

    FOR_EACH_ITEM_IN_MODULES(globals, global, modules, { wasm_set_symbol_index(&wasm, global, totalGlobals++); });

    FOR_EACH_ITEM_IN_MODULES(functions, function, modules,
                             { wasm_set_symbol_index(&wasm, function, totalFunctions++); });

    // now we actually emit the globals and functions

    FOR_EACH_ITEM_IN_MODULES(globals, global, modules, { wasm_emit_global(&wasm, global); });

    FOR_EACH_ITEM_IN_MODULES(functions, function, modules, { 
        wasm_emit_function(&wasm, function); 
    });

    // now write the sections into the final output stream
    error_t error = 0;

    // header first
    file_write(output, kWasmMagic, sizeof(kWasmMagic), &error);
    file_write(output, kWasmVersion, sizeof(kWasmVersion), &error);

    wasm_write_section(&wasm, WASM_GLOBAL_SECTION, wasm.totalGlobals, output);
    wasm_write_section(&wasm, WASM_EXPORT_SECTION, wasm.totalExports, output);
}
