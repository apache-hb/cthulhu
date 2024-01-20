#include "ref/sema.h"
#include "cthulhu/events/events.h"
#include "memory/memory.h"
#include "ref/ast.h"
#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

using namespace refl;

bool ResolveStack::enter_decl(Sema& sema, Decl *decl)
{
    if (auto it = std::find(m_stack.begin(), m_stack.end(), decl); it != m_stack.end())
    {
        event_builder_t event = sema.report(&kEvent_RecursiveEval, decl->get_node(), "recursive evaluation of %s", decl->get_name());
        for (auto it = m_stack.begin(); it != m_stack.end(); ++it)
        {
            Decl *decl = *it;
            msg_append(event, decl->get_node(), "  %s", decl->get_name());
        }
        return false;
    }

    m_stack.push_back(decl);
    return true;
}

void ResolveStack::leave_decl()
{
    CTASSERT(!m_stack.empty());
    m_stack.pop_back();
}

template<typename K, typename V>
static void map_foreach(map_t *map, auto&& fn)
{
    map_iter_t iter = map_iter(map);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        fn((K)entry.key, (V)entry.value);
    }
}

template<typename T>
static void vec_foreach(vector_t *vec, auto&& fn)
{
    for (size_t i = 0; i < vector_len(vec); ++i)
        fn((T)vector_get(vec, i));
}

declmap_t refl::get_builtin_types()
{
    declmap_t decls = {
        { "opaque", new MemoryType("opaque") },
        { "void", new VoidType("void") },
        { "string", new StringType("string") },
        { "bool", new BoolType("bool") },
        { "byte", new IntType("byte", eDigit8, eSignUnsigned) },

        { "int", new IntType("int", eDigitInt, eSignSigned) },
        { "uint", new IntType("uint", eDigitInt, eSignUnsigned) },

        { "int8", new IntType("int8", eDigit8, eSignSigned) },
        { "int16", new IntType("int16", eDigit16, eSignSigned) },
        { "int32", new IntType("int32", eDigit32, eSignSigned) },
        { "int64", new IntType("int64", eDigit64, eSignSigned) },

        { "uint8", new IntType("uint8", eDigit8, eSignUnsigned) },
        { "uint16", new IntType("uint16", eDigit16, eSignUnsigned) },
        { "uint32", new IntType("uint32", eDigit32, eSignUnsigned) },
        { "uint64", new IntType("uint64", eDigit64, eSignUnsigned) },

        { "usize", new IntType("usize", eDigit64, eSignUnsigned) },
        { "isize", new IntType("isize", eDigit64, eSignSigned) },

        { "float", new FloatType("float") },

        { "atomic", new TemplateAtomic("atomic") }
    };

    return decls;
}

void Sema::add_decl(const char *name, Decl *decl)
{
    CTASSERT(name != nullptr);
    CTASSERT(decl != nullptr);

    if (Decl *old = get_decl(name))
    {
        event_builder_t evt = report(&kEvent_SymbolShadowed, decl->get_node(), "duplicate declaration of %s", name);
        msg_append(evt, old->get_node(), "previous declaration");
    }
    else
    {
        m_decls[name] = decl;
    }
}

Decl *Sema::get_decl(const char *name) const
{
    if (auto it = m_decls.find(name); it != m_decls.end())
        return it->second;

    if (m_parent)
        return m_parent->get_decl(name);

    return nullptr;
}

void Sema::forward_module(ref_ast_t *mod)
{
    vec_foreach<ref_ast_t*>(mod->imports, [&](auto import) {
        imports.push_back(import->ident);
    });
    m_namespace = str_join("::", mod->mod, get_global_arena());
    vec_foreach<ref_ast_t*>(mod->decls, [&](auto decl) {
        forward_decl(decl->name, decl);
    });
}

Decl *Sema::forward_decl(const char *name, ref_ast_t *ast)
{
    switch (ast->kind) {
    case eAstClass: {
        Class *cls = new Class(ast);
        add_decl(name, cls);
        return cls;
    }
    case eAstStruct: {
        Struct *str = new Struct(ast);
        add_decl(name, str);
        return str;
    }
    case eAstVariant: {
        Variant *var = new Variant(ast);
        add_decl(name, var);
        return var;
    }
    default: return nullptr;
    }
}

void Sema::resolve_all()
{
    for (auto& [name, decl] : m_decls)
        resolve_decl(decl);
}

Type *Sema::resolve_type(ref_ast_t *ast)
{
    CTASSERT(ast != nullptr);
    switch (ast->kind)
    {
    case eAstName: {
        Decl *decl = get_decl(ast->ident);
        if (decl == nullptr)
        {
            report(&kEvent_SymbolNotFound, ast->node, "unresolved symbol '%s'", ast->ident);
            return nullptr;
        }
        decl->resolve_type(*this);
        CTASSERTF(decl->is_type(), "expected type, got %s", decl->get_name());
        CTASSERTF(decl->is_resolved(), "expected resolved type, got %s", decl->get_name());
        return static_cast<Type*>(decl);
    }
    case eAstInstance: {
        return resolve_generic(ast);
    }
    case eAstPointer: {
        Type *type = resolve_type(ast->ptr);
        if (type == nullptr)
        {
            report(&kEvent_InvalidType, ast->node, "invalid pointer type");
            return nullptr;
        }

        return new PointerType(ast->node, type);
    }
    case eAstOpaque: {
        return new OpaqueType(ast->node, ast->ident);
    }

    default: {
        report(&kEvent_InvalidType, ast->node, "invalid type");
        return nullptr;
    }
    }
}

Type *Sema::resolve_generic(ref_ast_t *ast)
{
    CTASSERT(ast != nullptr);

    Decl *decl = resolve_type(ast->generic);
    if (decl == nullptr)
    {
        report(&kEvent_SymbolNotFound, ast->node, "unresolved template type '%s'", ast->generic->ident);
        return nullptr;
    }

    if (!decl->is_template())
    {
        report(&kEvent_InvalidType, ast->node, "expected template type, got '%s'", decl->get_name());
        return nullptr;
    }

    TemplateType *tmpl = static_cast<TemplateType*>(decl);
    if (tmpl->get_params().size() != vector_len(ast->params))
    {
        event_builder_t evt = report(&kEvent_InvalidType, ast->node, "invalid number of template parameters for '%s'", decl->get_name());
        msg_note(evt, "expected %zu, got %zu", tmpl->get_params().size(), vector_len(ast->params));
        return nullptr;
    }

    std::vector<Type*> params;
    vec_foreach<ref_ast_t*>(ast->params, [&](auto param) {
        Type *type = resolve_type(param);
        if (type == nullptr)
        {
            report(&kEvent_InvalidType, param->node, "invalid template parameter");
            return;
        }
        params.push_back(type);
    });

    if (params.size() != vector_len(ast->params))
    {
        return nullptr;
    }

    return tmpl->instantiate(*this, params);
}

void Sema::emit_all(io_t *source, io_t *header, const char *file)
{
    // header preamble
    out_t h;
    h.writeln("#pragma once");
    h.writeln("// Generated from '{}'", file);
    h.writeln("// Dont edit this file, it will be overwritten on the next build");
    h.nl();
    h.writeln("#include \"reflect/reflect.h\"");
    h.nl();

    // source preamble
    out_t s;
    s.writeln("// Generated from '{}'", file);
    s.writeln("// Dont edit this file, it will be overwritten on the next build");
    s.nl();
    char *base = str_filename(io_name(header), get_global_arena());
    s.writeln("#include \"{}\"", base);
    s.nl();

    for (auto fd : imports)
    {
        if (fd[0] == '<')
        {
            h.writeln("#include {}", fd);
        }
        else
        {
            h.writeln("#include \"{}\"", fd);
        }
    }

    h.writeln("namespace {} {{", m_namespace);
    h.enter();

    h.writeln("// prototypes");

    for (auto& [name, decl] : m_decls)
        decl->emit_proto(h);

    h.nl();
    h.writeln("// implementation");

    DeclDepends depends;

    for (auto& [name, decl] : m_decls)
    {
        decl->get_deps(depends);
        depends.add(decl);
    }

    for (auto& decl : depends.m_depends)
    {
        CTASSERT(decl != nullptr);
        decl->emit_impl(h);
    }

    h.leave();
    h.writeln("}} // namespace {}", m_namespace);

    h.nl();
    h.writeln("namespace ctu {{");
    h.enter();
    h.writeln("// reflection");

    for (auto& decl : depends.m_depends)
    {
        decl->emit_reflection(*this, h);
    }

    h.leave();
    h.writeln("}} // namespace ctu");

    h.dump(header);
    s.dump(source);
}

void Field::resolve(Sema& sema)
{
    resolve_type(sema);
}

Field::Field(ref_ast_t *ast)
    : Decl(ast->node, eKindField, ast->name)
    , m_ast(ast)
{ }

void Field::resolve_type(Sema& sema)
{
    auto ty = sema.resolve_type(m_ast->type);
    if (ty == nullptr)
    {
        sema.report(&kEvent_InvalidType, m_ast->node, "invalid field type");
        return;
    }
    set_type(ty);
}

Case::Case(ref_ast_t *ast)
    : Decl(ast->node, eKindCase, ast->name)
    , m_ast(ast)
{ }

void Case::resolve(Sema&)
{
    if (is_resolved()) return;
    finish_resolve();

    mpz_init_set(m_value, m_ast->value);
    finish_resolve();
}

static ref_ast_t *get_attrib(vector_t *attribs, ref_kind_t kind)
{
    CTASSERT(attribs != nullptr);

    size_t len = vector_len(attribs);
    for (size_t i = 0; i < len; i++)
    {
        ref_ast_t *attrib = (ref_ast_t*)vector_get(attribs, i);
        if (attrib->kind == kind)
            return attrib;
    }
    return nullptr;
}

void Method::resolve(Sema& sema) {
    if (is_resolved()) return;
    finish_resolve();

    if (m_return != nullptr)
        m_return = sema.resolve_decl(m_return);

    if (m_ast->method_params != nullptr) {
        std::map<std::string, Param*> params;

        vec_foreach<ref_ast_t*>(m_ast->method_params, [&](auto param) {
            auto p = new Param(param);
            CTASSERTF(!params.contains(param->name), "duplicate parameter %s", param->name);
            params[param->name] = p;
            p->resolve(sema);
            m_params.push_back(p);
        });
    }

    ref_ast_t *cxxname = get_attrib(m_ast->attributes, eAstAttribCxxName);
    ref_ast_t *asserts = get_attrib(m_ast->attributes, eAstAttribAssert);

    m_thunk = (cxxname != nullptr) || (asserts != nullptr);
}

static const char *get_privacy(ref_privacy_t privacy)
{
    switch (privacy)
    {
    case ePrivacyPublic: return "public";
    case ePrivacyPrivate: return "private";
    case ePrivacyProtected: return "protected";
    default: NEVER("invalid privacy");
    }
}

void Method::emit_impl(out_t& out) const {
    Type *ret = m_return ? m_return->get_type() : new VoidType("void");
    auto it = ret->get_cxx_name(get_name());
    std::string params;
    std::string args;
    for (auto param : m_params)
    {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    }

    ref_ast_t *attrib = get_attrib(m_ast->attributes, eAstAttribCxxName);

    std::string inner = attrib ? attrib->ident : std::format("impl_{}", get_name());

    const char *privacy = ::get_privacy(m_ast->privacy);

    bool is_const = m_ast->const_method;

    if (m_thunk)
    {
        out.writeln("{}: {}({}) {}{{", privacy, it, params, is_const ? "const " : "");
        out.enter();
        out.writeln("return {}({});", inner, args);
        out.leave();
        out.writeln("}}");
    }
    else
    {
        out.writeln("{}: {}({}){};", privacy, it, params, is_const ? " const" : "");
    }
}

void Method::emit_method(out_t& out) const {
    Type *ret = m_return ? m_return->get_type() : new VoidType("void");
    auto it = ret->get_cxx_name(get_name());
    std::string params;
    std::string args;
    for (auto param : m_params)
    {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    }

    ref_ast_t *attrib = get_attrib(m_ast->attributes, eAstAttribCxxName);

    std::string inner = attrib ? attrib->ident : std::format("impl_{}", get_name());

    bool is_const = m_ast->const_method;

    if (m_thunk)
    {
        out.writeln("{}({}) {}{{", it, params, is_const ? "const " : "");
        out.enter();
        out.writeln("return {}({});", inner, args);
        out.leave();
        out.writeln("}}");
    }
    else
    {
        out.writeln("{}({}){};", it, params, is_const ? " const" : "");
    }
}

void Method::emit_thunk(out_t& out) const {
    Type *ret = m_return ? m_return->get_type() : new VoidType("void");
    ref_ast_t *attrib = get_attrib(m_ast->attributes, eAstAttribCxxName);
    std::string inner = attrib ? attrib->ident : std::format("impl_{}", get_name());
    auto it = ret->get_cxx_name(inner.c_str());
    std::string params;
    std::string args;
    for (auto param : m_params)
    {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    }

    out.writeln("{}({});", it, params);
}

Class::Class(ref_ast_t *ast)
    : Type(ast->node, eKindClass, ast->name)
    , m_ast(ast)
{ }

void Class::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();

    std::map<std::string, Field*> fields;
    std::map<std::string, Method*> methods;

    // TODO
    // std::map<const char*, GenericType*> tparams;

    if (m_ast->tparams != NULL && vector_len(m_ast->tparams) > 0)
    {
        NEVER("templates not implemented");
        // vec_foreach<ref_ast_t*>(m_ast->tparams, [&](auto param) {
        //     GenericType *p = new GenericType(param);
        //     p->resolve(sema);
        //     CTASSERTF(!tparams.contains(param->name), "duplicate type parameter %s", param->name);
        //     tparams[param->name] = p;

        //     m_tparams.push_back(p);
        // });
    }

    vec_foreach<ref_ast_t*>(m_ast->methods, [&](auto method) {
        Method *m = new Method(method);
        m->resolve(sema);
        CTASSERTF(!methods.contains(method->name), "duplicate method %s", method->name);
        methods[method->name] = m;

        m_methods.push_back(m);
    });

    vec_foreach<ref_ast_t*>(m_ast->fields, [&](auto field) {
        Field *f = new Field(field);
        f->resolve(sema);
        CTASSERTF(!fields.contains(field->name), "duplicate field %s", field->name);
        fields[field->name] = f;

        m_fields.push_back(f);
    });

    if (m_ast->parent != nullptr)
    {
        m_parent = sema.resolve_type(m_ast->parent);
        CTASSERTF(m_parent != nullptr, "invalid parent type");
        CTASSERTF(m_parent->get_kind() == eKindClass, "invalid parent type %s", m_parent->get_name());
    }

    finish_resolve();
}

Struct::Struct(ref_ast_t *ast)
    : Type(ast->node, eKindStruct, ast->name)
    , m_ast(ast)
{ }

void Struct::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();

    std::map<const char*, Field*> fields;

    vec_foreach<ref_ast_t*>(m_ast->fields, [&](auto field) {
        Field *f = new Field(field);
        f->resolve(sema);
        CTASSERTF(!fields.contains(field->name), "duplicate field %s", field->name);
        fields[field->name] = f;

        m_fields.push_back(f);
    });

    finish_resolve();
}

Variant::Variant(ref_ast_t *ast)
    : Type(ast->node, eKindStruct, ast->name)
    , m_ast(ast)
{ }

void Variant::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();

    std::map<const char*, Case*> cases;

    vec_foreach<ref_ast_t*>(m_ast->cases, [&](auto field) {
        Case *c = new Case(field);
        c->resolve(sema);
        CTASSERTF(!cases.contains(field->name), "duplicate case %s", field->name);
        cases[field->name] = c;

        m_cases.push_back(c);
    });

    if (m_ast->underlying)
    {
        m_underlying = sema.resolve_type(m_ast->underlying);
        CTASSERTF(m_underlying != nullptr, "invalid underlying type");
        CTASSERTF(m_underlying->get_kind() == eKindTypeInt, "invalid underlying type %s", m_underlying->get_name());
    }

    m_default_case = m_ast->default_case ? cases[m_ast->default_case->name] : nullptr;
}

static const char *digit_cxx_name(digit_t digit, sign_t sign)
{
    switch (digit)
    {
    case eDigitChar: return (sign == eSignUnsigned) ? "unsigned char" : "char";
    case eDigit8: return (sign == eSignUnsigned) ? "uint8_t" : "int8_t";
    case eDigit16: return (sign == eSignUnsigned) ? "uint16_t" : "int16_t";
    case eDigit32: return (sign == eSignUnsigned) ? "uint32_t" : "int32_t";
    case eDigit64: return (sign == eSignUnsigned) ? "uint64_t" : "int64_t";

    case eDigitInt: return (sign == eSignUnsigned) ? "unsigned int" : "int";
    case eDigitSize: return (sign == eSignUnsigned) ? "size_t" : "ptrdiff_t";

    default: NEVER("invalid digit");
    }
}

std::string IntType::get_cxx_name(const char *name) const
{
    const char *type = digit_cxx_name(m_digit, m_sign);
    return (name == nullptr) ? type : std::format("{} {}", type, name);
}

void Field::emit_impl(out_t& out) const
{
    const char *privacy = ::get_privacy(m_ast->privacy);
    auto it = get_type()->get_cxx_name(get_name());
    out.writeln("{}: {};", privacy, it);
}

void Field::emit_field(out_t& out) const
{
    auto it = get_type()->get_cxx_name(get_name());
    out.writeln("{};", it);
}

static void emit_record(out_t& out, ref_privacy_t privacy, const char *name, Type *parent, const char *ty, const std::vector<Field*>& fields, const std::vector<Method*>& methods)
{
    if (parent)
    {
        out.writeln("{} {} : public {} {{", ty, name, parent->get_name());
    }
    else
    {
        out.writeln("{} {} {{", ty, name);
    }
    out.enter();

    out.writeln("friend class ctu::TypeInfo<{}>;", name);

    out.writeln("// fields");

    for (auto field : fields)
    {
        ref_privacy_t field_privacy = field->get_privacy();
        if (field_privacy != privacy)
        {
            privacy = field_privacy;
            out.leave();
            out.writeln("{}:", get_privacy(field_privacy));
            out.enter();
        }

        field->emit_field(out);
    }

    out.writeln("// methods");

    for (auto method : methods)
    {
        ref_privacy_t field_privacy = method->get_privacy();
        if (field_privacy != privacy)
        {
            privacy = field_privacy;
            out.leave();
            out.writeln("{}:", get_privacy(field_privacy));
            out.enter();
        }

        method->emit_method(out);
    }

    out.writeln("// thunks");

    bool emit_private = false;
    for (auto method : methods)
    {
        if (!method->should_emit_thunk())
        {
            continue;
        }

        if (!emit_private)
        {
            emit_private = true;
            out.leave();
            out.writeln("private:");
            out.enter();
        }

        method->emit_thunk(out);
    }

    out.leave();
    out.writeln("}};");
}

void Class::emit_impl(out_t& out) const
{
    emit_record(out, ePrivacyPrivate, get_name(), m_parent, "class", m_fields, m_methods);
}

void Struct::emit_impl(out_t& out) const
{
    emit_record(out, ePrivacyPublic, get_name(), nullptr, "struct", m_fields, std::vector<Method*>());
}

void Case::emit_impl(out_t& out) const
{
    out.writeln("e{} = {},", get_name(), mpz_get_str(nullptr, 10, m_value));
}

void Variant::emit_proto(out_t& out) const
{
    out.writeln("class {};", get_name());
}

static void get_type_id(ref_ast_t *ast, mpz_t out)
{
    ref_ast_t *attrib = get_attrib(ast->attributes, eAstAttribTypeId);
    if (attrib)
    {
        mpz_init_set(out, attrib->id);
    }
    else
    {
        mpz_init_set_ui(out, str_hash(ast->name));
    }
}

void Variant::emit_impl(out_t& out) const
{
    auto under = m_underlying->get_cxx_name(nullptr);
    out.writeln("class {} {{", get_name());
    out.enter();
    out.writeln("friend class ctu::TypeInfo<{}>;", get_name());
    out.writeln("enum class inner_t : {} {{", under);
        out.enter();
        for (auto c : m_cases)
        {
            c->emit_impl(out);
        }
        out.leave();
    out.writeln("}};");
    out.nl();
    out.writeln("{} m_value;", under);
    out.nl();
    out.leave();
    out.writeln("public:");
    out.enter();
    out.writeln("constexpr {}({} value) : m_value(value) {{ }}", get_name(), under);
    out.writeln("constexpr {}(inner_t value) : m_value(({})value) {{ }}", get_name(), under);
    out.writeln("using enum inner_t;");
    out.nl();
    if (m_default_case)
    {
        out.writeln("static constexpr auto kDefaultCase = e{};", m_default_case->get_name());
        out.writeln("constexpr {}() : m_value(({})kDefaultCase) {{ }}", get_name(), under);
    }
    else
    {
        out.writeln("constexpr {}() = delete;", get_name());
    }

    out.writeln("constexpr operator inner_t() const {{ return (inner_t)m_value; }}", under);

    out.writeln("constexpr {}(const {}& other) : m_value(other.m_value) {{ }}", get_name(), get_name());
    out.writeln("constexpr {}& operator=(const {}& other) {{ m_value = other.m_value; return *this; }}", get_name(), get_name());

    out.writeln("constexpr {}(const {}&& other) : m_value(other.m_value) {{ }}", get_name(), get_name());
    out.writeln("constexpr {}& operator=(const {}&& other) {{ m_value = other.m_value; return *this; }}", get_name(), get_name());

    out.writeln("constexpr {} to_underlying() const {{ return m_value; }}", under);
    out.writeln("constexpr inner_t as_integral() const {{ return (inner_t)m_value; }}");

    out.nl();
    out.writeln("constexpr bool operator==(const {}& other) const {{ return m_value == other.m_value; }}", get_name());
    out.writeln("constexpr bool operator!=(const {}& other) const {{ return m_value != other.m_value; }}", get_name());

    bool is_bitflags = get_attrib(m_ast->attributes, eAstAttribBitflags) != nullptr;
    bool is_arithmatic = get_attrib(m_ast->attributes, eAstAttribArithmatic) != nullptr;
    bool is_iterator = get_attrib(m_ast->attributes, eAstAttribIterator) != nullptr;

    if (is_bitflags)
    {
        // emit bitwise operators
        out.nl();
        out.writeln("constexpr {} operator~() const {{ return {}(~m_value); }}", get_name(), get_name());
        out.writeln("constexpr {} operator|(const {}& other) const {{ return {}(m_value | other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {} operator&(const {}& other) const {{ return {}(m_value & other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {} operator^(const {}& other) const {{ return {}(m_value ^ other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {}& operator|=(const {}& other) {{ m_value |= other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator&=(const {}& other) {{ m_value &= other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator^=(const {}& other) {{ m_value ^= other.m_value; return *this; }}", get_name(), get_name());

        out.writeln("constexpr bool test({} other) const {{ return (m_value & other) != 0; }}", under);
        out.writeln("constexpr bool any({} other) const {{ return (m_value & other) != 0; }}", under);
        out.writeln("constexpr bool all({} other) const {{ return (m_value & other) == other; }}", under);
        out.writeln("constexpr bool none({} other) const {{ return (m_value & other) == 0; }}", under);
        out.writeln("constexpr {}& set({} other) {{ m_value |= other; return *this; }}", get_name(), under);
        out.writeln("constexpr {}& reset({} other) {{ m_value &= ~other; return *this; }}", get_name(), under);
        out.writeln("constexpr {}& flip({} other) {{ m_value ^= other; return *this; }}", get_name(), under);
    }

    if (is_arithmatic)
    {
        // implement arithmatic operators
        out.nl();
        out.writeln("constexpr {} operator+(const {}& other) const {{ return {}(m_value + other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {} operator-(const {}& other) const {{ return {}(m_value - other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {} operator*(const {}& other) const {{ return {}(m_value * other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {} operator/(const {}& other) const {{ return {}(m_value / other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {} operator%(const {}& other) const {{ return {}(m_value % other.m_value); }}", get_name(), get_name(), get_name());
        out.writeln("constexpr {}& operator+=(const {}& other) {{ m_value += other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator-=(const {}& other) {{ m_value -= other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator*=(const {}& other) {{ m_value *= other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator/=(const {}& other) {{ m_value /= other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator%=(const {}& other) {{ m_value %= other.m_value; return *this; }}", get_name(), get_name());
    }

    if (is_iterator)
    {
        // implement increment/decrement operators
        out.nl();
        out.writeln("constexpr {}& operator++() {{ ++m_value; return *this; }}", get_name());
        out.writeln("constexpr {} operator++(int) {{ return {}(m_value++); }}", get_name(), get_name());
        out.writeln("constexpr {}& operator--() {{ --m_value; return *this; }}", get_name());
        out.writeln("constexpr {} operator--(int) {{ return {}(m_value--); }}", get_name(), get_name());
    }

    out.leave();
    out.writeln("}};");
    out.writeln("static_assert(sizeof({}) == sizeof({}), \"{} size mismatch\");", get_name(), under, get_name());
}

static void emit_name_info(out_t& out, const std::string& id, ref_ast_t *ast)
{
    mpz_t typeid_value;
    get_type_id(ast, typeid_value);

    out.writeln("static constexpr ObjectName kFullName = impl::objname(\"{}\");", id);
    out.writeln("static constexpr ObjectName kName = impl::objname(\"{}\");", ast->name);
    out.writeln("static constexpr ObjectId kTypeId = {};", mpz_get_str(nullptr, 10, typeid_value));
}

static const char *access_name(ref_privacy_t privacy)
{
    switch (privacy)
    {
    case ePrivacyPublic: return "ePublic";
    case ePrivacyPrivate: return "ePrivate";
    case ePrivacyProtected: return "eProtected";
    default: NEVER("invalid privacy");
    }
}

static std::string attribs_name(ref_ast_t *ast)
{
    ref_ast_t *transient = get_attrib(ast->attributes, eAstAttribTransient);
    if (transient) return "eAttribTransient";

    return "eAttribNone";
}

static void emit_record_fields(out_t& out, const std::vector<Field*>& fields)
{
    out.writeln("static constexpr field_t kFields[{}] = {{", fields.size());
        out.enter();
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields[i];
            out.writeln("field_t {{");
            out.enter();
            out.writeln(".name    = impl::objname(\"{}\"),", f->get_name());
            out.writeln(".index   = {},", i);
            out.writeln(".access  = {},", access_name(f->get_privacy()));
            out.writeln(".attribs = {}", attribs_name(f->get_ast()));
            out.leave();
            out.writeln("}},");
        }
        out.leave();
    out.writeln("}};");
}

static void emit_record_visit(out_t& out, const std::string& id, const std::vector<Field*>& fields)
{
    out.writeln("constexpr auto visit_field({}& object, const field_t& field, auto&& fn) const {{", id);
    out.enter();
        out.writeln("switch (field.index) {{");
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields[i];
            out.writeln("case {}: return fn(object.{});", i, f->get_name());
        }
        out.writeln("default: return fn(ctu::OutOfBounds{{field.index}});");
        out.writeln("}}");
    out.leave();
    out.writeln("}};");
    out.nl();
    out.writeln("constexpr void foreach({}& object, auto&& fn) const {{", id);
    out.enter();
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields[i];
            out.writeln("fn(object.{});", f->get_name());
        }
    out.leave();
    out.writeln("}};");
}

static void emit_ctor(out_t& out)
{
    out.writeln("consteval TypeInfo() : TypeInfoBase(kName, sizeof(type_t), alignof(type_t), kTypeId) {{ }}");
}

static void emit_reflect_hook(out_t& out, const std::string& id)
{
    out.writeln("template<> consteval auto reflect<{}>() {{", id);
    out.enter();
    out.writeln("return TypeInfo<{}>{{}};", id, id);
    out.leave();
    out.writeln("}}");
}

static void emit_info_header(out_t& out, const std::string& id)
{
    out.writeln("template<> class TypeInfo<{}> : TypeInfoBase {{", id);
    out.writeln("public:");
}

void Struct::emit_reflection(Sema& sema, out_t& out) const
{
    if (ref_ast_t *noreflect = get_attrib(m_ast->attributes, eAstAttribNoReflect); noreflect)
    {
        return;
    }

    auto id = std::format("{}::{}", sema.get_namespace(), get_name());

    mpz_t typeid_value;
    get_type_id(m_ast, typeid_value);

    emit_info_header(out, id);
        out.enter();
        out.writeln("using type_t = {};", id);
        out.writeln("using field_t = ctu::ObjectField;");
        out.nl();
        emit_name_info(out, id, m_ast);
        out.nl();
        emit_record_fields(out, m_fields);
        out.nl();
        emit_ctor(out);
        out.nl();
        emit_record_visit(out, id, m_fields);
        out.leave();
    out.writeln("}};");
    out.nl();
    emit_reflect_hook(out, id);
    out.nl();
}

void Class::emit_reflection(Sema& sema, out_t& out) const
{
    if (ref_ast_t *noreflect = get_attrib(m_ast->attributes, eAstAttribNoReflect); noreflect)
    {
        return;
    }

    auto id = std::format("{}::{}", sema.get_namespace(), get_name());
    auto parent = m_parent ? m_parent->get_cxx_name(nullptr) : "void";

    mpz_t typeid_value;
    get_type_id(m_ast, typeid_value);

    emit_info_header(out, id);
        out.enter();
        out.writeln("using type_t = {};", id);
        out.writeln("using super_t = {};", parent);
        out.writeln("using field_t = ctu::ObjectField;");
        out.writeln("using method_t = ctu::ObjectMethod;");
        out.nl();
        emit_name_info(out, id, m_ast);
        out.writeln("static constexpr bool kHasSuper = {};", m_parent != nullptr);
        out.writeln("static constexpr TypeInfo<{}> kSuper{{}};", parent);
        out.nl();
        emit_record_fields(out, m_fields);
        out.nl();
        out.writeln("// methods");
        out.writeln("static constexpr method_t kMethods[{}] = {{", m_methods.size());
            out.enter();
            for (size_t i = 0; i < m_methods.size(); ++i)
            {
                auto m = m_methods[i];
                out.writeln("method_t {{ .name = impl::objname(\"{}\"), .index = {} }},", m->get_name(), i);
            }
            out.leave();
        out.writeln("}};");
        emit_ctor(out);
        out.nl();
        emit_record_visit(out, id, m_fields);
        out.leave();
    out.writeln("}};");
    out.nl();
    emit_reflect_hook(out, id);
    out.nl();
}

void Variant::emit_reflection(Sema& sema, out_t& out) const
{
    if (ref_ast_t *noreflect = get_attrib(m_ast->attributes, eAstAttribNoReflect); noreflect)
    {
        return;
    }

    auto id = std::format("{}::{}", sema.get_namespace(), get_name());
    auto underlying = m_underlying->get_cxx_name(nullptr);

    mpz_t typeid_value;
    get_type_id(m_ast, typeid_value);

    emit_info_header(out, id);
        out.enter();
        out.writeln("using type_t = {};", id);
        out.writeln("using underlying_t = {};", underlying);
        out.writeln("using case_t = ctu::EnumCase<{}>;", id);
        out.nl();
        emit_name_info(out, id, m_ast);
        if (m_underlying)
            out.writeln("static constexpr TypeInfo<{}> kUnderlying{{}};", underlying);
        else
            out.writeln("static TypeInfo<void> kUnderlying{{}};");

        out.writeln("static constexpr bool kHasDefault = {};", m_default_case != nullptr);
        if (m_default_case)
            out.writeln("static constexpr {} kDefaultCase = {}::e{};", id, id, m_default_case->get_name());

        out.nl();
        out.writeln("static constexpr case_t kCases[{}] = {{", m_cases.size());
            out.enter();
            for (auto c : m_cases)
                out.writeln("case_t {{ impl::objname(\"e{}\"), {}::e{} }},", c->get_name(), id, c->get_name());
            out.leave();
        out.writeln("}};");
        out.nl();
        emit_ctor(out);
        out.nl();

        out.writeln("constexpr bool is_valid(underlying_t value) const {{");
        out.enter();
            out.writeln("for (auto option : kCases) {{");
            out.enter();
            out.writeln("if (option.value.to_underlying() == value) return true;");
            out.leave();
            out.writeln("}}");
            out.writeln("return false;");
        out.leave();
        out.writeln("}};");
        out.nl();
        out.writeln("constexpr ObjectName to_string(type_t value) const {{");
        out.enter();
            out.writeln("for (auto option : kCases) {{");
            out.enter();
            out.writeln("if (option.value == value) return option.name;");
            out.leave();
            out.writeln("}}");
            out.writeln("return impl::kUnknown;");
        out.leave();
        out.writeln("}};");
    out.leave();
    out.writeln("}};");

    out.nl();
    emit_reflect_hook(out, id);
    out.nl();
}
