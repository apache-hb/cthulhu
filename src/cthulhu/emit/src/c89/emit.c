#include "base/util.h"
#include "c89.h"

#include "cthulhu/events/events.h"
#include "io/io.h"
#include "notify/notify.h"
#include "scan/node.h"
#include "std/str.h"
#include "std/map.h"
#include "std/set.h"
#include "std/vector.h"
#include "std/typed/vector.h"

#include "fs/fs.h"

#include "arena/arena.h"

#include "base/panic.h"
#include "core/macros.h"

#include <string.h>

static c89_source_t *source_new(io_t *io, const char *path, arena_t *arena)
{
    c89_source_t *source = ARENA_MALLOC(sizeof(c89_source_t), path, io, arena);
    source->io = io;
    source->path = path;
    return source;
}

static c89_source_t *header_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    char *it = str_format(emit->arena, "include/%s.h", path);
    fs_file_create(emit->fs, it);

    io_t *io = fs_open(emit->fs, it, eAccessWrite);
    c89_source_t *source = source_new(io, str_format(emit->arena, "%s.h", path), emit->arena);
    map_set(emit->hdrmap, mod, source);
    return source;
}

static c89_source_t *source_for(c89_emit_t *emit, const ssa_module_t *mod, const char *path)
{
    char *it = str_format(emit->arena, "src/%s.c", path);
    fs_file_create(emit->fs, it);

    io_t *io = fs_open(emit->fs, it, eAccessWrite);
    c89_source_t *source = source_new(io, it, emit->arena);
    map_set(emit->srcmap, mod, source);
    return source;
}

c89_source_t *c89_get_source(c89_emit_t *emit, const ssa_module_t *mod)
{
    // simcoe: nasty hack to implement single output quicker quicker
    if (emit->file_override) { return &emit->source_override; }

    c89_source_t *src = map_get(emit->srcmap, mod);
    CTASSERTF(src != NULL, "no source for %s", mod->name);

    return src;
}

c89_source_t *c89_get_header(c89_emit_t *emit, const ssa_module_t *mod)
{
    if (emit->file_override) { return &emit->header_override; }

    c89_source_t *src = map_get(emit->hdrmap, mod);
    CTASSERTF(src != NULL, "no header for %s", mod->name);

    return src;
}

io_t *c89_get_header_io(c89_emit_t *emit, const ssa_module_t *mod)
{
    c89_source_t *src = c89_get_header(emit, mod);
    return src->io;
}

io_t *c89_get_source_io(c89_emit_t *emit, const ssa_module_t *mod)
{
    c89_source_t *src = c89_get_source(emit, mod);
    return src->io;
}

char *get_namespace(const ssa_module_t *mod, arena_t *arena)
{
    char *path = str_join("::", mod->path, arena);
    return str_replace(path, "-", "_", arena);
}

// begin api

static const char *format_path(const char *base, const char *name, arena_t *arena)
{
    if (ctu_strlen(base) == 0) { return name; }
    return str_format(arena, "%s/%s", base, name);
}

static void collect_deps(c89_emit_t *emit, const ssa_module_t *mod, vector_t *symbols)
{
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(symbols, i);
        map_set(emit->modmap, global, (ssa_module_t*)mod);
    }
}

static void c89_begin_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    // collect all symbols defined in this module
    collect_deps(emit, mod, mod->globals);
    collect_deps(emit, mod, mod->functions);

    // create source and header files
    char *path = begin_module(&emit->emit, emit->fs, mod); // lets not scuff the original path

    const char *src_file = format_path(path, mod->name, emit->arena);
    const char *hdr_file = format_path(path, mod->name, emit->arena);

    vector_push(&emit->sources, str_format(emit->arena, "src/%s.c", src_file));

    c89_source_t *src = source_for(emit, mod, src_file);
    c89_source_t *hdr = header_for(emit, mod, hdr_file);

    write_string(hdr->io, "#pragma once\n");
    write_string(hdr->io, "#include <stdbool.h>\n");
    write_string(hdr->io, "#include <stdint.h>\n");

    write_string(src->io, "#include \"%s.h\"\n", hdr_file);
}

// emit api

static const char *format_c89_link(tree_link_t linkage)
{
    switch (linkage)
    {
    case eLinkImport: return "extern ";
    case eLinkModule: return "static ";

    case eLinkExport:
    case eLinkEntryCli:
    case eLinkEntryGui:
        return "";

    default: NEVER("unknown linkage %d", linkage);
    }
}

static void get_required_headers(c89_emit_t *emit, set_t *requires, const ssa_module_t *root, vector_t *symbols)
{
    size_t len = vector_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *global = vector_get(symbols, i);
        set_t *deps = map_get(emit->deps, global);
        if (deps == NULL) { continue; }

        set_iter_t iter = set_iter(deps);
        while (set_has_next(&iter))
        {
            const ssa_symbol_t *dep = set_next(&iter);
            const ssa_module_t *dep_mod = map_get(emit->modmap, dep);
            if (dep_mod != root)
            {
                set_add(requires, dep_mod);
            }
        }
    }
}

static void emit_required_headers(c89_emit_t *emit, const ssa_module_t *mod)
{
    // TODO: this is very coarse, we should only add deps to the headers
    // for symbols that are externally visible

    size_t len = vector_len(mod->globals);
    set_t *requires = set_new(CT_MAX(len, 1), kTypeInfoPtr, emit->arena); // set of modules required by this module

    get_required_headers(emit, requires, mod, mod->globals);
    get_required_headers(emit, requires, mod, mod->functions);

    io_t *hdr = c89_get_header_io(emit, mod);
    set_iter_t iter = set_iter(requires);
    while (set_has_next(&iter))
    {
        const ssa_module_t *item = set_next(&iter);
        c89_source_t *dep = c89_get_header(emit, item);
        write_string(hdr, "#include \"%s\"\n", dep->path);
    }
}

static const char *mangle_symbol_name(const ssa_symbol_t *symbol)
{
    switch (symbol->linkage)
    {
    case eLinkEntryCli: return "main";
    case eLinkEntryGui: return "WinMain";
    default: break;
    }

    if (symbol->link_name != NULL) { return symbol->link_name; }
    return symbol->name;
}

static bool is_entry_point(tree_link_t link)
{
    return link == eLinkEntryCli || link == eLinkEntryGui;
}

static const char *format_symbol(c89_emit_t *emit, const ssa_type_t *type, const char *name)
{
    return c89_format_type(emit, type, name, eFormatEmitConst);
}

static const char *format_symbol_cxx(c89_emit_t *emit, const ssa_type_t *type, const char *name)
{
    return c89_format_type(emit, type, name, eFormatEmitCxx);
}

static void define_enum(io_t *io, const ssa_type_t *type, c89_emit_t *emit)
{
    // update c89_format_type eTypeEnum when this is changed
    const ssa_type_enum_t it = type->sum;
    size_t len = typevec_len(it.cases);
    const ssa_type_t *underlying = it.underlying;
    io_printf(io, "#if defined(CTU_CINTERFACE) || !defined(__cplusplus)\n");
    char *under = str_format(emit->arena, "%s_underlying_t", type->name);
    const char *tydef = c89_format_type(emit, underlying, under, false);
    write_string(io, "typedef %s;\n", tydef);

    write_string(io, "enum %s_cases_t { /* %zu cases */\n", type->name, len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_case_t *field = typevec_offset(it.cases, i);

        // TODO: formalize the name mangling for enum fields
        write_string(io, "\te%s%s = %s,\n", type->name, field->name, mpz_get_str(NULL, 10, field->value));
    }
    write_string(io, "};\n");
    io_printf(io, "#endif /* CTU_CINTERFACE */\n");

    ssa_module_t *mod = map_get(emit->modmap, type);
    char *ns = get_namespace(mod, emit->arena);

    write_string(io, "#ifdef __cplusplus\n");
    io_printf(io, "namespace %s {\n", ns);
    const char *under_cxx = c89_format_type(emit, underlying, NULL, false);
    write_string(io, "\tenum class %s : %s {\n", type->name, under_cxx);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_case_t *field = typevec_offset(it.cases, i);
        write_string(io, "\t\te%s = %s,\n", field->name, mpz_get_str(NULL, 10, field->value));
    }
    write_string(io, "\t};\n");
    io_printf(io, "} /* %s */\n", ns);
    write_string(io, "#endif /* __cplusplus */ \n");
}

static void c89_proto_aggregate(c89_emit_t *emit, io_t *io, const char *ty, const char *name, const ssa_module_t *mod)
{
    char *ns = get_namespace(mod, emit->arena);
    io_printf(io, "#ifdef __cplusplus\n");
    io_printf(io, "namespace %s {\n", ns);
    write_string(io, "\t%s %s;\n", ty, name);
    io_printf(io, "} /* %s */\n", ns);
    io_printf(io, "#endif /* __cplusplus */ \n");

    write_string(io, "#if defined(CTU_CINTERFACE) || !defined(__cplusplus)\n");
    write_string(io, "%s %s;\n", ty, name);
    write_string(io, "#endif /* CTU_CINTERFACE */\n");
}

void c89_proto_type(c89_emit_t *emit, io_t *io, const ssa_type_t *type)
{
    ssa_module_t *mod = map_get(emit->modmap, type);
    switch (type->kind)
    {
    case eTypeStruct:
        c89_proto_aggregate(emit, io, "struct", type->name, mod);
        break;
    case eTypeUnion:
        c89_proto_aggregate(emit, io, "union", type->name, mod);
        break;

    case eTypeEnum:
        define_enum(io, type, emit);
        break;

    default:
        break; // TODO: how should we honor visiblity and aliases?
    }
}

static void write_global(c89_emit_t *emit, io_t *io, const ssa_symbol_t *global)
{
    const char *it = c89_format_storage(emit, global->storage, mangle_symbol_name(global));
    const char *link = format_c89_link(global->linkage);

    write_string(io, "%s%s", link, it);
}

void c89_proto_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *global)
{
    if (global->visibility == eVisiblePublic)
    {
        CTASSERT(global->linkage != eLinkModule); // TODO: move this check into the checker

        io_t *io = c89_get_header_io(emit, mod);
        write_global(emit, io, global);
        write_string(io, ";\n");
    }
    else
    {
        io_t *io = c89_get_source_io(emit, mod);
        write_global(emit, io, global);
        write_string(io, ";\n");
    }
}

void c89_proto_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *func)
{
    // dont generate prototypes for entry points
    if (is_entry_point(func->linkage)) { return; }

    io_t *src = c89_get_source_io(emit, mod);
    io_t *hdr = c89_get_header_io(emit, mod);

    const ssa_type_t *type = func->type;
    CTASSERTF(type->kind == eTypeClosure, "expected closure type on %s, got %d", func->name, type->kind);

    ssa_type_closure_t closure = type->closure;
    const char *params = c89_format_params(emit, closure.params, closure.variadic);
    const char *result = format_symbol(emit, closure.result, mangle_symbol_name(func));

    const char *link = format_c89_link(func->linkage);

    io_t *dst = func->visibility == eVisiblePublic ? hdr : src;
    write_string(dst, "%s%s(%s);\n", link, result, params);
}

static void proto_symbols(c89_emit_t *emit, const ssa_module_t *mod, vector_t *vec, void (*fn)(c89_emit_t*, const ssa_module_t*, const ssa_symbol_t*))
{
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *symbol = vector_get(vec, i);
        fn(emit, mod, symbol);
    }
}

static void c89_proto_types(c89_emit_t *emit, io_t *io, vector_t *types)
{
    size_t len = vector_len(types);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_type_t *type = vector_get(types, i);
        c89_proto_type(emit, io, type);
    }
}

static void c89_proto_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    emit_required_headers(emit, mod);

    io_t *hdr = c89_get_header_io(emit, mod);
    size_t len = vector_len(mod->types);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_type_t *type = vector_get(mod->types, i);
        c89_proto_type(emit, hdr, type);
    }

    proto_symbols(emit, mod, mod->globals, c89_proto_global);
    proto_symbols(emit, mod, mod->functions, c89_proto_function);
}

static void reflect_enum(c89_emit_t *emit, io_t *io, const char *ns, const ssa_type_t *type)
{
    ssa_type_enum_t it = type->sum;
    size_t len = typevec_len(it.cases);

    // simcoe: make this based around decorators in the language frontends.
    //         use a skeleton file like flex/bison do and fill that out instead of this mess.

    io_printf(io, "template<> class ctu::ReflectObject<%s::%s> : public ctu::TypeInfo {\n", ns, type->name);
    io_printf(io, "\tusing super_t = ctu::TypeInfo;\n");
    io_printf(io, "public:\n"); // type aliases
    io_printf(io, "\tusing type_t = %s::%s;\n", ns, type->name);
    io_printf(io, "\tusing case_t = ctu::EnumCase<type_t>;\n");
    io_printf(io, "\tusing underlying_t = %s;\n", c89_format_type(emit, it.underlying, NULL, false));
    io_printf(io, "\n"); // constructors and getters
    io_printf(io, "\tconsteval ReflectObject() noexcept : super_t(\"%s::%s\") { }\n\n", ns, type->name);
    io_printf(io, "\tconstexpr static underlying_t to_underlying(type_t value) noexcept { return static_cast<underlying_t>(value); }\n");
    io_printf(io, "\tconstexpr static type_t from_underlying(underlying_t value) noexcept { return static_cast<type_t>(value); }\n\n");
    io_printf(io, "\tconstexpr std::span<const case_t> get_cases() const noexcept { return kCaseData; }\n");
    io_printf(io, "\nprivate:\n");
    io_printf(io, "\tstatic constexpr std::array<case_t, %zu> kCaseData = {\n", len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_case_t *field = typevec_offset(it.cases, i);
        io_printf(io, "\t\tcase_t { \"e%s\", type_t::e%s },\n", field->name, field->name);
    }
    io_printf(io, "\t};\n");
    io_printf(io, "};\n\n");

    io_printf(io, "template<> struct ctu::ReflectInfo<%s::%s> {\n\tusing reflect_t = impl::TypeInfoHandle<%s::%s>;\n};\n\n", ns, type->name, ns, type->name);
    io_printf(io, "template<>\n");
    io_printf(io, "consteval auto ctu::reflect<%s::%s>() noexcept {\n", ns, type->name);
    io_printf(io, "\treturn ctu::ReflectInfo<%s::%s>{};\n", ns, type->name);
    io_printf(io, "}\n\n");
}

static void reflect_struct(c89_emit_t *emit, io_t *io, const char *ns, const ssa_type_t *type)
{
    CT_UNUSED(emit);

    ssa_type_record_t it = type->record;
    size_t len = typevec_len(it.fields);

    io_printf(io, "template<> class ctu::impl::TypeInfoHandle<%s::%s> : public ctu::TypeInfo {\n", ns, type->name);
    io_printf(io, "\tusing super_t = ctu::TypeInfo;\n");
    io_printf(io, "public:\n"); // type aliases
    io_printf(io, "\tusing type_t = %s::%s;\n", ns, type->name);
    io_printf(io, "\tusing field_t = ctu::RecordField;\n");
    io_printf(io, "\n"); // constructors and getters
    io_printf(io, "\tconsteval TypeInfoHandle() noexcept : super_t(\"%s::%s\") { }\n\n", ns, type->name);
    io_printf(io, "\tconstexpr std::span<const field_t> get_fields() const noexcept { return kFieldData; }\n\n");
    io_printf(io, "\tconstexpr auto visit(type_t& object, size_t index, auto&& visitor) const noexcept {\n");
    io_printf(io, "\t\tswitch (index) {\n");
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(it.fields, i);
        io_printf(io, "\t\tcase %zu: return visitor(object.%s);\n", i, field->name);
    }
    io_printf(io, "\t\tdefault: CTU_UNREACHABLE();\n");
    io_printf(io, "\t\t}\n");
    io_printf(io, "\t}\n\n");
    io_printf(io, "\tconstexpr auto visit(const type_t& object, size_t index, auto&& visitor) const noexcept {\n");
    io_printf(io, "\t\tswitch (index) {\n");
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(it.fields, i);
        io_printf(io, "\t\tcase %zu: return visitor(object.%s);\n", i, field->name);
    }
    io_printf(io, "\t\tdefault: CTU_UNREACHABLE();\n");
    io_printf(io, "\t\t}\n");
    io_printf(io, "\t}\n");
    io_printf(io, "\nprivate:\n");
    io_printf(io, "\tstatic constexpr std::array<field_t, %zu> kFieldData = {\n", len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(it.fields, i);
        io_printf(io, "\t\tfield_t { \"%s\" },\n", field->name);
    }
    io_printf(io, "\t};\n");
    io_printf(io, "};\n\n");

    io_printf(io, "template<> struct ctu::ReflectInfo<%s::%s> {\n\tusing reflect_t = impl::TypeInfoHandle<%s::%s>;\n};\n\n", ns, type->name, ns, type->name);
    io_printf(io, "template<>\n");
    io_printf(io, "consteval auto ctu::reflect<%s::%s>() noexcept {\n", ns, type->name);
    io_printf(io, "\treturn ctu::ReflectInfo<%s::%s>{};\n", ns, type->name);
    io_printf(io, "}\n\n");
}

static void emit_type_info(c89_emit_t *emit, io_t *io, const char *ns, const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypeEnum:
        reflect_enum(emit, io, ns, type);
        break;

    case eTypeStruct:
        reflect_struct(emit, io, ns, type);
        break;

    default:
        break;
    }
}

static void emit_reflect_guard_begin(io_t *io)
{
    io_printf(io, "#if CTU_CXX_REFLECT\n");
}

static void emit_reflect_guard_end(io_t *io)
{
    io_printf(io, "#endif /* CTU_CXX_REFLECT */\n");
}

static void emit_reflect_hooks(c89_emit_t *emit, io_t *io, const ssa_module_t *mod)
{
    size_t len = vector_len(mod->types);

    if (len == 0) return;

    char *ns = get_namespace(mod, emit->arena);

    for (size_t i = 0; i < len; i++)
    {
        const ssa_type_t *type = vector_get(mod->types, i);
        emit_type_info(emit, io, ns, type);
    }
}

/// bbs

static const ssa_type_t *get_operand_type(c89_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandImm: {
        const ssa_value_t *value = operand.value;
        return value->type;
    }
    case eOperandLocal: {
        const ssa_local_t *local = typevec_offset(emit->current->locals, operand.local);
        return local->type;
    }
    case eOperandParam: {
        const ssa_param_t *param = typevec_offset(emit->current->params, operand.param);
        return param->type;
    }
    case eOperandGlobal: {
        const ssa_symbol_t *global = operand.global;
        return global->type;
    }
    case eOperandFunction: {
        const ssa_symbol_t *func = operand.function;
        return func->type;
    }
    case eOperandReg: {
        const ssa_block_t *bb = operand.vreg_context;
        const ssa_step_t *step = typevec_offset(bb->steps, operand.vreg_index);
        const ssa_type_t *type = map_get(emit->stepmap, step);
        return type;
    }

    default: NEVER("unknown operand kind %d", operand.kind);
    }
}

static void set_step_type(c89_emit_t *emit, const ssa_step_t *step, const ssa_type_t *type)
{
    map_set(emit->stepmap, step, (ssa_type_t*)type);
}

static const char *c89_name_vreg(c89_emit_t *emit, const ssa_step_t *step, const ssa_type_t *type)
{
    set_step_type(emit, step, (ssa_type_t*)type);

    const char *id = str_format(emit->arena, "vreg%s", get_step_name(&emit->emit, step));
    return format_symbol(emit, type, id);
}

static const char *c89_name_vreg_by_operand(c89_emit_t *emit, const ssa_step_t *step, ssa_operand_t operand)
{
    const ssa_type_t *type = get_operand_type(emit, operand);
    return c89_name_vreg(emit, step, type);
}

static const ssa_type_t *get_reg_type(const ssa_type_t *type)
{
    switch (type->kind)
    {
    case eTypePointer: {
        ssa_type_pointer_t pointer = type->pointer;
        return pointer.pointer;
    }

    default: NEVER("expected storage or pointer type, got %s (on %s)", ssa_type_name(type->kind), type->name);
    }
}

static const char *c89_name_load_vreg_by_operand(c89_emit_t *emit, const ssa_step_t *step, ssa_operand_t operand)
{
    const ssa_type_t *type = get_operand_type(emit, operand);
    return c89_name_vreg(emit, step, get_reg_type(type));
}

static const char *operand_type_string(c89_emit_t *emit, ssa_operand_t operand)
{
    const ssa_type_t *type = get_operand_type(emit, operand);
    return type_to_string(type, emit->arena);
}

static const char *c89_format_value(c89_emit_t *emit, const ssa_value_t* value);

static const char *c89_format_pointer(c89_emit_t *emit, vector_t *data)
{
    arena_t *arena = emit->arena;
    size_t len = vector_len(data);
    vector_t *result = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_value_t *value = vector_get(data, i);
        const char *it = c89_format_value(emit, value);
        vector_set(result, i, (char*)it);
    }

    char *joined = str_join(", ", result, arena);
    return str_format(arena, "{ %s }", joined);
}

static const char *c89_format_value(c89_emit_t *emit, const ssa_value_t* value)
{
    const ssa_type_t *type = value->type;
    switch (type->kind)
    {
    case eTypeBool: return value->bool_value ? "true" : "false";
    case eTypeDigit: return mpz_get_str(NULL, 10, value->digit_value);
    case eTypePointer: return c89_format_pointer(emit, value->data);
    default: NEVER("unknown type kind %d", type->kind);
    }
}

static const char *c89_format_local(c89_emit_t *emit, size_t local)
{
    typevec_t *locals = emit->current->locals;
    CTASSERTF(local < typevec_len(locals), "local(%zu) > locals(%zu)", local, typevec_len(locals));

    const ssa_local_t *it = typevec_offset(locals, local);
    return str_format(emit->arena, "l_%s", it->name);
}

static const char *c89_format_param(c89_emit_t *emit, size_t param)
{
    typevec_t *params = emit->current->params;
    CTASSERTF(param < typevec_len(params), "param(%zu) > params(%zu)", param, typevec_len(params));

    const ssa_param_t *it = typevec_offset(params, param);
    return it->name;
}

static const char *c89_format_operand(c89_emit_t *emit, ssa_operand_t operand)
{
    switch (operand.kind)
    {
    case eOperandEmpty: return "/* empty */";
    case eOperandImm:
        return c89_format_value(emit, operand.value);

    case eOperandBlock:
        return str_format(emit->arena, "bb%s", get_block_name(&emit->emit, operand.bb));

    case eOperandReg:
        return str_format(emit->arena, "vreg%s", get_step_from_block(&emit->emit, operand.vreg_context, operand.vreg_index));

    case eOperandGlobal:
        return mangle_symbol_name(operand.global);

    case eOperandFunction:
        return mangle_symbol_name(operand.function);

    case eOperandLocal:
        return c89_format_local(emit, operand.local);

    case eOperandParam:
        return c89_format_param(emit, operand.param);

    default: NEVER("unknown operand kind %d", operand.kind);
    }
}

static bool operand_is_empty(ssa_operand_t operand)
{
    return operand.kind == eOperandEmpty;
}

static bool operand_cant_return(ssa_operand_t operand)
{
    if (operand.kind == eOperandImm)
    {
        const ssa_value_t *value = operand.value;
        const ssa_type_t *type = value->type;
        return type->kind == eTypeUnit || type->kind == eTypeEmpty;
    }

    return operand.kind == eOperandEmpty;
}

static void c89_write_address(c89_emit_t *emit, io_t *io, const ssa_step_t *step)
{
    ssa_addr_t addr = step->addr;
    ssa_operand_t symbol = addr.symbol;
    const ssa_type_t *type = get_operand_type(emit, symbol);

    const ssa_type_t *ptr = ssa_type_pointer(type->name, eQualNone, (ssa_type_t*)type, 0);
    const char *step_name = c89_name_vreg(emit, step, ptr);

    write_string(io, "\t%s = &(%s); /* %s */\n",
        step_name,
        c89_format_operand(emit, addr.symbol),
        type_to_string(ptr, emit->arena)
    );
}

static void c89_write_offset(c89_emit_t *emit, io_t *io, const ssa_step_t *step)
{
    ssa_offset_t offset = step->offset;
    write_string(io, "\t%s = &%s[%s]; /* (array = %s, offset = %s) */\n",
        c89_name_vreg_by_operand(emit, step, offset.array),
        c89_format_operand(emit, offset.array),
        c89_format_operand(emit, offset.offset),
        operand_type_string(emit, offset.array),
        operand_type_string(emit, offset.offset)
    );
}

static void c89_write_member(c89_emit_t *emit, io_t *io, const ssa_step_t *step)
{
    CTASSERTF(step->opcode == eOpMember, "expected member, got %s", ssa_opcode_name(step->opcode));

    ssa_member_t member = step->member;
    const ssa_type_t *record_ptr = get_operand_type(emit, member.object);
    CTASSERTF(record_ptr->kind == eTypePointer, "expected record type, got %s", type_to_string(record_ptr, emit->arena));

    ssa_type_pointer_t ptr = record_ptr->pointer;
    const ssa_type_t *record = ptr.pointer;
    CTASSERTF(record->kind == eTypeStruct, "expected record type, got %s", type_to_string(record, emit->arena));

    ssa_type_record_t record_type = record->record;
    const ssa_field_t *field = typevec_offset(record_type.fields, member.index);

    write_string(io, "\t%s = &%s->%s;\n",
        c89_name_vreg(emit, step, ssa_type_pointer(field->name, eQualNone, (ssa_type_t*)field->type, 1)),
        c89_format_operand(emit, member.object),
        field->name
    );
}

static void c89_write_block(c89_emit_t *emit, io_t *io, const ssa_block_t *bb)
{
    size_t len = typevec_len(bb->steps);
    write_string(io, "bb%s: { /* len = %zu */\n", get_block_name(&emit->emit, bb), len);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_step_t *step = typevec_offset(bb->steps, i);
        switch (step->opcode)
        {
        case eOpStore: {
            ssa_store_t store = step->store;
            write_string(io, "\t*(%s) = %s;\n",
                c89_format_operand(emit, store.dst),
                c89_format_operand(emit, store.src)
            );
            break;
        }
        case eOpCast: {
            ssa_cast_t cast = step->cast;
            write_string(io, "\t%s = (%s)(%s);\n",
                c89_name_vreg(emit, step, cast.type),
                format_symbol(emit, cast.type, NULL),
                c89_format_operand(emit, cast.operand)
            );
            break;
        }
        case eOpLoad: {
            ssa_load_t load = step->load;
            write_string(io, "\t%s = *(%s);\n",
                c89_name_load_vreg_by_operand(emit, step, load.src),
                c89_format_operand(emit, load.src)
            );
            break;
        }

        case eOpAddress:
            c89_write_address(emit, io, step);
            break;
        case eOpOffset:
            c89_write_offset(emit, io, step);
            break;
        case eOpMember:
            c89_write_member(emit, io, step);
            break;

        case eOpUnary: {
            ssa_unary_t unary = step->unary;
            write_string(io, "\t%s = (%s %s);\n",
                c89_name_vreg_by_operand(emit, step, unary.operand),
                unary_symbol(unary.unary),
                c89_format_operand(emit, unary.operand)
            );
            break;
        }
        case eOpBinary: {
            ssa_binary_t bin = step->binary;
            write_string(io, "\t%s = (%s %s %s);\n",
                c89_name_vreg_by_operand(emit, step, bin.lhs),
                c89_format_operand(emit, bin.lhs),
                binary_symbol(bin.binary),
                c89_format_operand(emit, bin.rhs)
            );
            break;
        }
        case eOpCompare: {
            ssa_compare_t cmp = step->compare;
            write_string(io, "\t%s = (%s %s %s);\n",
                c89_name_vreg(emit, step, ssa_type_bool("bool", eQualConst)),
                c89_format_operand(emit, cmp.lhs),
                compare_symbol(cmp.compare),
                c89_format_operand(emit, cmp.rhs)
            );
            break;
        }

        case eOpCall: {
            ssa_call_t call = step->call;
            size_t args_len = typevec_len(call.args);

            const ssa_type_t *ty = get_operand_type(emit, call.function);
            ssa_type_closure_t closure = ty->closure;
            const ssa_type_t *result = closure.result;

            vector_t *args = vector_of(args_len, emit->arena);
            for (size_t arg_idx = 0; arg_idx < args_len; arg_idx++)
            {
                const ssa_operand_t *operand = typevec_offset(call.args, arg_idx);
                vector_set(args, arg_idx, (char*)c89_format_operand(emit, *operand));
            }

            write_string(io, "\t");

            if (result->kind != eTypeEmpty && result->kind != eTypeUnit)
            {
                write_string(io, "%s = ", c89_name_vreg(emit, step, result));
            }

            write_string(io, "%s(%s);\n",
                c89_format_operand(emit, call.function),
                str_join(", ", args, emit->arena)
            );
            break;
        }

        case eOpJump: {
            ssa_jump_t jmp = step->jump;
            write_string(io, "\tgoto %s;\n", c89_format_operand(emit, jmp.target));
            break;
        }
        case eOpBranch: {
            ssa_branch_t br = step->branch;
            write_string(io, "\tif (%s) { goto %s; }", c89_format_operand(emit, br.cond), c89_format_operand(emit, br.then));
            if (!operand_is_empty(br.other))
            {
                write_string(io, " else { goto %s; }", c89_format_operand(emit, br.other));
            }
            write_string(io, "\n");
            break;
        }
        case eOpReturn: {
            ssa_return_t ret = step->ret;
            if (!operand_cant_return(ret.value))
            {
                write_string(io, "\treturn %s;\n", c89_format_operand(emit, ret.value));
            }
            else
            {
                write_string(io, "\treturn;\n");
            }
            break;
        }

        default: NEVER("unknown opcode %d", step->opcode);
        }
    }
    write_string(io, "} /* end %s */\n", get_block_name(&emit->emit, bb));
}

/// defines

static void define_record(c89_emit_t *emit, const char *aggregate, io_t *io, const ssa_type_t *type, const char *ns)
{
    const ssa_type_record_t record = type->record;
    io_printf(io, "#if defined(CTU_CINTERFACE) || !defined(__cplusplus)\n");
    io_printf(io, "%s %s {\n", aggregate, type->name);
    size_t len = typevec_len(record.fields);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_field_t *field = typevec_offset(record.fields, i);
        io_printf(io, "\t%s;\n", format_symbol(emit, field->type, field->name));
    }
    io_printf(io, "};\n");
    io_printf(io, "#endif /* CTU_CINTERFACE */\n");

    if (ns != NULL)
    {
        io_printf(io, "#ifdef __cplusplus\n");
        io_printf(io, "namespace %s {\n", ns);

        io_printf(io, "\t%s %s {\n", aggregate, type->name);
        for (size_t i = 0; i < len; i++)
        {
            const ssa_field_t *field = typevec_offset(record.fields, i);
            io_printf(io, "\t\t%s;\n", format_symbol_cxx(emit, field->type, field->name));
        }
        io_printf(io, "\t};\n");

        io_printf(io, "} /* %s */\n", ns);
        io_printf(io, "#endif /* __cplusplus */ \n");
    }
}

void c89_define_type(c89_emit_t *emit, io_t *io, const ssa_type_t *type, const char *ns)
{
    switch (type->kind)
    {
    case eTypeStruct:
        define_record(emit, "struct", io, type, ns);
        break;
    case eTypeUnion:
        define_record(emit, "union", io, type, ns);
        break;

    default:
        break;
    }
}

static void write_init(c89_emit_t *emit, io_t *io, const ssa_value_t *value)
{
    const ssa_type_t *type = value->type;
    const char *init = c89_format_value(emit, value);

    if (type->kind == eTypePointer)
    {
        write_string(io, " = %s", init);
    }
    else
    {
       write_string(io, " = { %s }", init);
    }
}

void c89_define_global(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *symbol)
{
    io_t *src = c89_get_source_io(emit, mod);

    if (symbol->linkage != eLinkImport)
    {
        const ssa_value_t *value = symbol->value;
        write_global(emit, src, symbol);
        if (value->init)
        {
            write_init(emit, src, value);
        }

        write_string(src, ";\n");
    }
}

static void write_locals(c89_emit_t *emit, io_t *io, typevec_t *locals)
{
    size_t len = typevec_len(locals);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_local_t *local = typevec_offset(locals, i);
        write_string(io, "\t%s;\n",
            c89_format_storage(emit, local->storage, str_format(emit->arena, "l_%s", local->name))
        );
    }
}

void c89_define_function(c89_emit_t *emit, const ssa_module_t *mod, const ssa_symbol_t *func)
{
    io_t *src = c89_get_source_io(emit, mod);

    const ssa_type_t *type = func->type;
    CTASSERTF(type->kind == eTypeClosure, "expected closure type on %s, got %d", func->name, type->kind);

    ssa_type_closure_t closure = type->closure;
    const char *params = c89_format_params(emit, closure.params, closure.variadic);
    const char *result = format_symbol(emit, closure.result, mangle_symbol_name(func));

    const char *link = format_c89_link(func->linkage);

    if (func->linkage != eLinkImport)
    {
        write_string(src, "%s%s(%s) {\n", link, result, params);
        write_locals(emit, src, func->locals);
        write_string(src, "\tgoto bb%s;\n", get_block_name(&emit->emit, func->entry));
        size_t len = vector_len(func->blocks);
        for (size_t i = 0; i < len; i++)
        {
            const ssa_block_t *bb = vector_get(func->blocks, i);
            c89_write_block(emit, src, bb);
        }
        write_string(src, "}\n");

        map_reset(emit->stepmap);
        counter_reset(&emit->emit);
    }
}

static void define_symbols(c89_emit_t *emit, const ssa_module_t *mod, vector_t *vec, void (*fn)(c89_emit_t*, const ssa_module_t*, const ssa_symbol_t*))
{
    size_t len = vector_len(vec);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_symbol_t *symbol = vector_get(vec, i);
        emit->current = symbol;
        fn(emit, mod, symbol);
    }
}

static void define_type_ordererd(c89_emit_t *emit, io_t *io, const ssa_type_t *type)
{
    if (set_contains(emit->defined, type)) { return; }
    set_add(emit->defined, type);

    // TODO: this is probably a touch broken, types may be put into the wrong translation
    if (type->kind == eTypeStruct || type->kind == eTypeUnion)
    {
        // only match struct and union as they can have dependencies
        ssa_type_record_t record = type->record;
        size_t len = typevec_len(record.fields);
        for (size_t i = 0; i < len; i++)
        {
            const ssa_field_t *field = typevec_offset(record.fields, i);
            define_type_ordererd(emit, io, field->type);
        }
    }

    const ssa_module_t *mod = map_get(emit->modmap, type);
    if (mod != NULL)
    {
        const char *ns = get_namespace(mod, emit->arena);
        c89_define_type(emit, io, type, ns);
    }
    else
    {
        c89_define_type(emit, io, type, NULL);
    }
}

static void c89_define_types(c89_emit_t *emit, io_t *io, vector_t *types)
{
    size_t len = vector_len(types);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_type_t *type = vector_get(types, i);
        define_type_ordererd(emit, io, type);
    }
}

static void c89_define_module(c89_emit_t *emit, const ssa_module_t *mod)
{
    io_t *hdr = c89_get_header_io(emit, mod);
    c89_define_types(emit, hdr, mod->types);
    define_symbols(emit, mod, mod->globals, c89_define_global);
    define_symbols(emit, mod, mod->functions, c89_define_function);
}

static void c89_emit_single(c89_emit_t *emit, vector_t *mods)
{
    io_t *hdr = emit->header_override.io;
    io_t *src = emit->source_override.io;

    // generate prelude
    // TODO: this should be done in one place
    io_printf(hdr, "#pragma once\n");
    io_printf(hdr, "/* generated by cthulhu */\n");
    io_printf(hdr, "/* do not modify this file directly */\n");
    io_printf(hdr, "#include <stdbool.h>\n");
    io_printf(hdr, "#include <stdint.h>\n");
    io_printf(hdr, "#if __has_include(\"ctu/reflect.h\")\n");
    io_printf(hdr, "#   include \"ctu/reflect.h\"\n");
    io_printf(hdr, "#endif /* __has_include(\"ctu/reflect.h\") */\n");

    io_printf(src, "/* generated by cthulhu */\n");
    io_printf(src, "/* do not modify this file directly */\n");
    char *header = str_filename(emit->header_override.path, emit->arena);
    io_printf(src, "#include \"%s\"\n", header);

    size_t len = vector_len(mods);
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(mods, i);
        // we're doing everything in a single source file, so we can do all the type ordering at once
        // we also know theres no required headers, so we can skip that step

        c89_proto_types(emit, hdr, mod->types);
    }

    // now we define all the types in the header
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(mods, i);

        c89_define_types(emit, hdr, mod->types);
    }

    // define prototypes for all globals and functions
    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(mods, i);

        proto_symbols(emit, mod, mod->globals, c89_proto_global);
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(mods, i);

        proto_symbols(emit, mod, mod->functions, c89_proto_function);
    }

    if (emit->emit_reflect)
    {
        emit_reflect_guard_begin(hdr);
        for (size_t i = 0; i < len; i++)
        {
            const ssa_module_t *mod = vector_get(mods, i);
            emit_reflect_hooks(emit, hdr, mod);
        }
        emit_reflect_guard_end(hdr);
    }
}

static void add_module_deps(c89_emit_t *emit, const ssa_module_t *mod, vector_t *deps)
{
    size_t len = vector_len(deps);
    for (size_t i = 0; i < len; i++)
    {
        const void *dep = vector_get(deps, i);
        map_set(emit->modmap, dep, (void*)mod);
    }
}

static void add_all_module_deps(c89_emit_t *emit, const ssa_module_t *mod)
{
    add_module_deps(emit, mod, mod->types);
    add_module_deps(emit, mod, mod->functions);
    add_module_deps(emit, mod, mod->globals);
}

c89_emit_result_t emit_c89(const c89_emit_options_t *options)
{
    emit_options_t opts = options->opts;
    size_t len = vector_len(opts.modules);
    c89_emit_result_t result = { 0 };

    arena_t *arena = opts.arena;

    c89_emit_t emit = {
        .arena = arena,
        .emit = {
            .arena = arena,
            .reports = opts.reports,
            .block_names = names_new(64, arena),
            .vreg_names = names_new(64, arena),
        },
        .modmap = map_optimal(len * 2, kTypeInfoPtr, arena),
        .srcmap = map_optimal(len, kTypeInfoPtr, arena),
        .hdrmap = map_optimal(len, kTypeInfoPtr, arena),

        .stepmap = map_optimal(64, kTypeInfoPtr, arena),
        .defined = set_new(64, kTypeInfoPtr, arena),

        .fs = opts.fs,
        .deps = opts.deps,
        .sources = vector_new(32, opts.arena),

        .emit_reflect = options->emit_reflect_info,
    };

    const char *header_out = options->output_header;
    const char *source_out = options->output_source;

    if ((header_out != NULL) ^ (source_out != NULL))
    {
        msg_notify(opts.reports, &kEvent_SourceAndHeaderOutput, node_builtin(), "both or neither source and header must be specified");
        goto cleanup;
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        add_all_module_deps(&emit, mod);
    }

    if (header_out != NULL && source_out != NULL)
    {
        emit.file_override = true;

        fs_file_create(opts.fs, header_out);
        io_t *header_io = fs_open(opts.fs, header_out, eAccessWrite);

        fs_file_create(opts.fs, source_out);
        io_t *source_io = fs_open(opts.fs, source_out, eAccessWrite);

        c89_source_t header = {
            .io = header_io,
            .path = header_out,
        };

        c89_source_t source = {
            .io = source_io,
            .path = source_out,
        };

        emit.header_override = header;
        emit.source_override = source;

        c89_emit_single(&emit, opts.modules);
    }
    else
    {
        // simcoe: use the default module generator only when manual output is not specified
        for (size_t i = 0; i < len; i++)
        {
            const ssa_module_t *mod = vector_get(opts.modules, i);
            c89_begin_module(&emit, mod);
        }

        for (size_t i = 0; i < len; i++)
        {
            const ssa_module_t *mod = vector_get(opts.modules, i);
            c89_proto_module(&emit, mod);
        }

        if (options->emit_reflect_info)
        {
            for (size_t i = 0; i < len; i++)
            {
                const ssa_module_t *mod = vector_get(opts.modules, i);
                io_t *hdr = c89_get_header_io(&emit, mod);
                emit_reflect_guard_begin(hdr);
                emit_reflect_hooks(&emit, hdr, mod);
                emit_reflect_guard_end(hdr);
            }
        }
    }

    for (size_t i = 0; i < len; i++)
    {
        const ssa_module_t *mod = vector_get(opts.modules, i);
        c89_define_module(&emit, mod);
    }

cleanup:
    result.sources = emit.sources;
    return result;
}
