#include "cthulhu/emit/emit.h"
#include "cthulhu/hlir/query.h"

typedef struct
{
    reports_t *reports;
    file_t file;

    uint32_t totalFunctions;
    uint32_t totalExportedFunctions;
    uint32_t totalImportedFunctions;

    stream_t *typeSection;
    stream_t *functionSection;
    stream_t *exportSection;
} wasm_t;

static const char *wasm_mangle_name(const hlir_t *hlir)
{
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

static void wasm_emit_global(wasm_t *wasm, const hlir_t *hlir)
{
    UNUSED(wasm);

    const char *name = wasm_mangle_name(hlir);
    logverbose("emitting global %s", name);
}

static const uint8_t kFuncType = 0x60;

static void wasm_write_array_length(stream_t *stream, uint32_t length)
{
    stream_write_bytes(stream, &length, sizeof(length));
}

static uint8_t wasm_encode_value_type(wasm_t *wasm, const hlir_t *hlir)
{
    hlir_kind_t kind = get_hlir_kind(hlir);
    const node_t *node = get_hlir_node(hlir);
    if (kind != HLIR_DIGIT)
    {
        report(wasm->reports, ERROR, node, "unsupported value type %s", hlir_kind_to_string(kind));
        return 0x00;
    }

    switch (hlir->width)
    {
    case DIGIT_CHAR: case DIGIT_SHORT:
    case DIGIT_INT:
        return 0x7F;
    case DIGIT_LONG:
        return 0x7E;

    default:
        report(wasm->reports, ERROR, node, "unsupported digit type %s", hlir_digit_to_string(kind));
        return 0x00;
    }
}

static void wasm_emit_function(wasm_t *wasm, const hlir_t *hlir)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    wasm->totalFunctions++;

    switch (attribs->linkage)
    {
    case LINK_IMPORTED:
        wasm->totalImportedFunctions++;
        break;
    case LINK_EXPORTED:
        wasm->totalExportedFunctions++;
        break;

    default:
        break;
    }

    const char *name = wasm_mangle_name(hlir);
    logverbose("emitting function %s", name);

    vector_t *params = closure_params(hlir);
    const hlir_t *result = closure_result(hlir);

    size_t totalParams = vector_len(params);

    stream_write_bytes(wasm->typeSection, &kFuncType, sizeof(kFuncType));
    wasm_write_array_length(wasm->typeSection, totalParams);
    
    for (size_t i = 0; i < totalParams; i++)
    {
        hlir_t *param = vector_get(params, i);
        uint8_t valueType = wasm_encode_value_type(wasm, param);
        stream_write_bytes(wasm->typeSection, &valueType, sizeof(valueType));
    }

    hlir_kind_t resultKind = get_hlir_kind(result);

    if (resultKind != HLIR_VOID)
    {
        wasm_write_array_length(wasm->typeSection, 1);
        uint8_t valueType = wasm_encode_value_type(wasm, result);
        stream_write_bytes(wasm->typeSection, &valueType, sizeof(valueType));
    }
    else
    {
        wasm_write_array_length(wasm->typeSection, 0);
    }
}

static const uint8_t kWasmMagic[] = { '\0', 'a', 's', 'm' };
static const uint8_t kWasmVersion[] = { 1, 0, 0, 0 };

static void wasm_write_section(file_t file, stream_t *stream, uint8_t type)
{
    error_t error = 0;
    uint32_t size = stream_len(stream);

    file_write(file, &type, sizeof(type), &error);
    file_write(file, &size, sizeof(size), &error);
    file_write(file, stream_data(stream), size, &error);
}

void wasm_emit_modules(reports_t *reports, vector_t *modules, file_t output)
{
    wasm_t wasm = {
        .reports = reports,
        .file = output,

        .typeSection = stream_new(0x1000),
        .functionSection = stream_new(0x1000),
        .exportSection = stream_new(0x1000),
    };

    size_t totalModules = vector_len(modules);

    for (size_t i = 0; i < totalModules; i++)
    {
        hlir_t *mod = vector_get(modules, i);
        vector_t *globals = mod->globals;

        for (size_t j = 0; j < vector_len(globals); j++)
        {
            hlir_t *global = vector_get(globals, j);
            wasm_emit_global(&wasm, global);
        }
    }

    for (size_t i = 0; i < totalModules; i++) 
    {
        hlir_t *mod = vector_get(modules, i);
        vector_t *functions = mod->functions;
        
        for (size_t j = 0; j < vector_len(functions); j++)
        {
            hlir_t *function = vector_get(functions, j);
            wasm_emit_function(&wasm, function);
        }
    }

    error_t error = 0;

    file_write(output, kWasmMagic, sizeof(kWasmMagic), &error);
    file_write(output, kWasmVersion, sizeof(kWasmVersion), &error);

    wasm_write_section(output, wasm.typeSection, 1);
    wasm_write_section(output, wasm.functionSection, 3);
    wasm_write_section(output, wasm.exportSection, 7);
}
