#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/util/map.h"

#include <string.h>

/**
 * this file is based off the <a href="https://webassembly.github.io/spec/core/binary/index.html">wasm binary format specification</a>
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

    map_t *symbolIndices;

    size_t totalGlobals;

    stream_t *sections[WASM_SECTION_TOTAL];
} wasm_t;

typedef struct
{
    wasm_t *wasm;

    wasm_section_t section;
} wasm_stream_t;

#define WASM_CONST 0x00
#define WASM_MUT 0x01

#define WASM_NUMTYPE_I32 0x7F
#define WASM_NUMTYPE_I64 0x7E

#define WASM_REFTYPE_FUNCREF 0x70
#define WASM_REFTYPE_EXTERNREF 0x6F

#define WASM_INST_I32_CONST 0x41
#define WASM_INST_I64_CONST 0x42

#define WASM_INST_END 0x0B

#define WASM_EXPORTDESC_FUNC 0x00
#define WASM_EXPORTDESC_TABLE 0x01
#define WASM_EXPORTDESC_MEMORY 0x02
#define WASM_EXPORTDESC_GLOBAL 0x03

typedef struct {
    uint8_t buffer[16];
    size_t length;
} leb128_t;

static leb128_t ui_leb128(uint64_t value)
{
    leb128_t result = { 0 };

    bool more = true;
    do {
        uint8_t c = value & 0x7F;
        value >>= 7;
        more = value != 0;

        result.buffer[result.length++] = c | (more ? 0x80 : 0);
    } while (more);

    return result;
}

static leb128_t si_leb128(int64_t value)
{
    leb128_t result = { 0 };

    bool more = true;
    do {
        uint8_t c = value & 0x7F;
        value >>= 7;
        more = c & 0x40 ? value != -1 : value != 0;

        result.buffer[result.length++] = c | (more ? 0x80 : 0);
    } while (more);

    return result;
}

static void wasm_set_symbol_index(wasm_t *wasm, const hlir_t *symbol, uintptr_t index)
{
    map_set_ptr(wasm->symbolIndices, symbol, (void *)index);
}

static void wasm_write_to_section(wasm_t *wasm, wasm_section_t section, const void *data, size_t size)
{
    stream_write_bytes(wasm->sections[section], data, size);
}

static void wasm_stream_write(wasm_stream_t *stream, const void *data, size_t size)
{
    wasm_write_to_section(stream->wasm, stream->section, data, size);
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

static void wasm_emit_expr(wasm_stream_t *stream, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    switch (kind)
    {
    case HLIR_DIGIT_LITERAL:
        wasm_emit_digit_literal(stream, hlir);
        break;

    default:
        report(stream->wasm->reports, INTERNAL, get_hlir_node(hlir), "unsupported expression %s", hlir_kind_to_string(kind));
        break;
    }
}

// https://webassembly.github.io/spec/core/binary/types.html#binary-globaltype
static void wasm_emit_global_type(wasm_stream_t *stream, const hlir_t *hlir)
{
    const hlir_t *type = get_hlir_type(hlir);
    const hlir_attributes_t *attrbs = get_hlir_attributes(type);
    uint8_t mut = wasm_get_mut(stream->wasm, hlir, attrbs->tags);
    uint8_t valtype = wasm_get_valtype(stream->wasm, type);

    uint8_t vals[] = {valtype, mut};

    wasm_stream_write(stream, vals, sizeof(vals));
}

static void wasm_emit_global(wasm_t *wasm, const hlir_t *hlir)
{
    wasm_stream_t stream = {.wasm = wasm, .section = WASM_GLOBAL_SECTION};
    wasm_emit_global_type(&stream, hlir);
    wasm_emit_expr(&stream, hlir->value);
    
    wasm_write_op(&stream, WASM_INST_END);

    wasm->totalGlobals += 1;
}

static void wasm_write_section(wasm_t *wasm, wasm_section_t section, uint32_t entries, file_t file)
{
    stream_t *stream = wasm->sections[section];
    uint32_t size = (uint32_t)stream_len(stream);
    uint8_t sec = (uint8_t)section;

    logverbose("writing section %d (%d bytes)", section, size);

    error_t error = 0;

    file_write(file, &sec, sizeof(sec), &error);

    leb128_t actualLength = ui_leb128(size + 1);
    file_write(file, actualLength.buffer, actualLength.length, &error);

    leb128_t actualEntries = ui_leb128(entries);
    file_write(file, actualEntries.buffer, actualEntries.length, &error);

    file_write(file, stream_data(stream), size, &error);
}

void wasm_emit_modules(reports_t *reports, vector_t *modules, file_t output)
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

    FOR_EACH_ITEM_IN_MODULES(globals, global, modules, { 
        wasm_set_symbol_index(&wasm, global, totalGlobals++); 
    });

    FOR_EACH_ITEM_IN_MODULES(functions, function, modules, { 
        wasm_set_symbol_index(&wasm, function, totalFunctions++); 
    });

    // now we actually emit the globals and functions

    FOR_EACH_ITEM_IN_MODULES(globals, global, modules, { 
        wasm_emit_global(&wasm, global); 
    });

#if 0
    FOR_EACH_ITEM_IN_MODULES(functions, function, modules, { 
        wasm_emit_function(&wasm, function); 
    });
#endif

    // now write the sections into the final output stream
    error_t error = 0;

    // header first
    file_write(output, kWasmMagic, sizeof(kWasmMagic), &error);
    file_write(output, kWasmVersion, sizeof(kWasmVersion), &error);

    wasm_write_section(&wasm, WASM_GLOBAL_SECTION, wasm.totalGlobals, output);
}
