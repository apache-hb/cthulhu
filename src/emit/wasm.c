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

    stream_t *sections[WASM_SECTION_TOTAL];
} wasm_t;

static void wasm_set_symbol_index(wasm_t *wasm, const hlir_t *symbol, uintptr_t index)
{
    map_set_ptr(wasm->symbolIndices, symbol, (void *)index);
}

static void wasm_write_to_section(wasm_t *wasm, wasm_section_t section, const void *data, size_t size)
{
    stream_write_bytes(wasm->sections[section], data, size);
}

static void wasm_write_result_type(wasm_t *wasm, wasm_section_t section, const hlir_t *type)
{
    
}

static void wasm_emit_global(wasm_t *wasm, const hlir_t *hlir)
{

}

static void wasm_emit_function(wasm_t *wasm, const hlir_t *hlir)
{

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

    FOR_EACH_ITEM_IN_MODULES(functions, function, modules, { 
        wasm_emit_function(&wasm, function); 
    });

    // now write the sections into the final output stream
    error_t error = 0;

    // header first
    file_write(output, kWasmMagic, sizeof(kWasmMagic), &error);
    file_write(output, kWasmVersion, sizeof(kWasmVersion), &error);
}
