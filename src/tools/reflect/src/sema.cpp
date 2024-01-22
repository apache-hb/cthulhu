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
    if (m_stack.find(decl) != SIZE_MAX)
    {
        event_builder_t event = sema.report(&kEvent_RecursiveEval, decl->get_node(), "recursive evaluation of %s", decl->get_name());
        m_stack.foreach([&](auto decl) {
            msg_append(event, decl->get_node(), "  %s", decl->get_name());
        });
        return false;
    }

    m_stack.push(decl);
    return true;
}

void ResolveStack::leave_decl()
{
    CTASSERT(m_stack.size());
    m_stack.pop();
}

template<typename T>
static void vec_foreach(vector_t *vec, auto&& fn)
{
    for (size_t i = 0; i < vector_len(vec); ++i)
        fn((T)vector_get(vec, i));
}

declmap_t refl::get_builtin_types()
{
    declmap_t decls { 64, get_global_arena() };

    decls.set("opaque", new MemoryType("opaque"));
    decls.set("void", new VoidType("void"));
    decls.set("string", new StringType("string"));
    decls.set("bool", new BoolType("bool"));
    decls.set("byte", new IntType("byte", eDigit8, eSignUnsigned));
    decls.set("int", new IntType("int", eDigitInt, eSignSigned));
    decls.set("uint", new IntType("uint", eDigitInt, eSignUnsigned));
    decls.set("long", new IntType("long", eDigitLong, eSignSigned));
    decls.set("ulong", new IntType("ulong", eDigitLong, eSignUnsigned));
    decls.set("int8", new IntType("int8", eDigit8, eSignSigned));
    decls.set("int16", new IntType("int16", eDigit16, eSignSigned));
    decls.set("int32", new IntType("int32", eDigit32, eSignSigned));
    decls.set("int64", new IntType("int64", eDigit64, eSignSigned));
    decls.set("uint8", new IntType("uint8", eDigit8, eSignUnsigned));
    decls.set("uint16", new IntType("uint16", eDigit16, eSignUnsigned));
    decls.set("uint32", new IntType("uint32", eDigit32, eSignUnsigned));
    decls.set("uint64", new IntType("uint64", eDigit64, eSignUnsigned));
    decls.set("fast8", new IntType("intfast8", eDigitFast8, eSignSigned));
    decls.set("fast16", new IntType("intfast16", eDigitFast16, eSignSigned));
    decls.set("fast32", new IntType("intfast32", eDigitFast32, eSignSigned));
    decls.set("fast64", new IntType("intfast64", eDigitFast64, eSignSigned));
    decls.set("ufast8", new IntType("uintfast8", eDigitFast8, eSignUnsigned));
    decls.set("ufast16", new IntType("uintfast16", eDigitFast16, eSignUnsigned));
    decls.set("ufast32", new IntType("uintfast32", eDigitFast32, eSignUnsigned));
    decls.set("ufast64", new IntType("uintfast64", eDigitFast64, eSignUnsigned));
    decls.set("least8", new IntType("intleast8", eDigitLeast8, eSignSigned));
    decls.set("least16", new IntType("intleast16", eDigitLeast16, eSignSigned));
    decls.set("least32", new IntType("intleast32", eDigitLeast32, eSignSigned));
    decls.set("least64", new IntType("intleast64", eDigitLeast64, eSignSigned));
    decls.set("uleast8", new IntType("uintleast8", eDigitLeast8, eSignUnsigned));
    decls.set("uleast16", new IntType("uintleast16", eDigitLeast16, eSignUnsigned));
    decls.set("uleast32", new IntType("uintleast32", eDigitLeast32, eSignUnsigned));
    decls.set("uleast64", new IntType("uintleast64", eDigitLeast64, eSignUnsigned));
    decls.set("intptr", new IntType("intptr", eDigitPtr, eSignSigned));
    decls.set("uintptr", new IntType("uintptr", eDigitPtr, eSignUnsigned));
    decls.set("usize", new IntType("usize", eDigitSize, eSignUnsigned));
    decls.set("isize", new IntType("isize", eDigitSize, eSignSigned));
    decls.set("float", new FloatType("float"));
    decls.set("atomic", new TemplateAtomic("atomic"));
    decls.set("const", new TemplateConst("const"));

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
        m_decls.set(name, decl);
    }
}

Decl *Sema::get_decl(const char *name) const
{
    if (Decl* it = m_decls.get(name))
        return it;

    if (m_parent)
        return m_parent->get_decl(name);

    return nullptr;
}

void Sema::forward_module(ref_ast_t *mod)
{
    if (mod->api) m_api = mod->api;

    vec_foreach<ref_ast_t*>(mod->imports, [&](auto import) {
        imports.push(import->ident);
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
    m_decls.foreach([&](auto, auto decl) {
        resolve_decl(decl);
    });
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

    Vector<Type*> params { vector_len(ast->params), get_global_arena() };
    vec_foreach<ref_ast_t*>(ast->params, [&](auto param) {
        Type *type = resolve_type(param);
        if (type == nullptr)
        {
            report(&kEvent_InvalidType, param->node, "invalid template parameter");
            return;
        }
        params.push(type);
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
    out_t h = this;
    h.writeln("#pragma once");
    h.writeln("// Generated from '{}'", file);
    h.writeln("// Dont edit this file, it will be overwritten on the next build");
    h.nl();
    h.writeln("#include \"reflect/reflect.h\"");
    h.nl();

    // source preamble
    out_t s = this;
    s.writeln("// Generated from '{}'", file);
    s.writeln("// Dont edit this file, it will be overwritten on the next build");
    s.nl();
    char *base = str_filename(io_name(header), get_global_arena());
    s.writeln("#include \"{}\"", base);
    s.nl();

    imports.foreach([&](auto fd) {
        if (fd[0] == '<')
        {
            h.writeln("#include {}", fd);
        }
        else
        {
            h.writeln("#include \"{}\"", fd);
        }
    });

    h.writeln("namespace {} {{", m_namespace);
    h.enter();

    h.writeln("// prototypes");

    m_decls.foreach([&](auto, auto decl) {
        decl->emit_proto(h);
    });

    h.nl();
    h.writeln("// implementation");

    DeclDepends depends { { 64, get_global_arena() } };

    m_decls.foreach([&](auto, auto decl) {
        decl->get_deps(depends);
        depends.add(decl);
    });

    depends.m_depends.foreach([&](auto decl) {
        CTASSERT(decl != nullptr);
        decl->emit_impl(h);
    });

    h.leave();
    h.writeln("}} // namespace {}", m_namespace);

    h.nl();
    h.writeln("namespace ctu {{");
    h.enter();
    h.writeln("// reflection");

    depends.m_depends.foreach([&](auto decl) {
        CTASSERT(decl != nullptr);
        decl->emit_reflection(*this, h);
    });


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

    finish_resolve();
}

std::string Case::get_value() const {
    CTASSERT(m_ast->value != nullptr);
    switch (m_ast->value->kind)
    {
    case eAstName: return m_ast->value->ident;
    case eAstInteger: return mpz_get_str(nullptr, 10, m_ast->value->integer);

    default: NEVER("invalid case value %d", m_ast->value->kind);
    }
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
        Map<const char*, Param*> params { vector_len(m_ast->method_params), get_global_arena() };

        vec_foreach<ref_ast_t*>(m_ast->method_params, [&](auto param) {
            auto p = new Param(param);
            CTASSERTF(!params.get(param->name), "duplicate parameter %s", param->name);
            params.set(param->name, p);
            p->resolve(sema);
            m_params.push(p);
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
    default: NEVER("invalid privacy %d", privacy);
    }
}

void Method::emit_impl(out_t& out) const {
    Type *ret = m_return ? m_return->get_type() : new VoidType("void");
    auto it = ret->get_cxx_name(get_name());
    std::string params;
    std::string args;
    m_params.foreach([&](auto param) {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    });

    ref_ast_t *attrib = get_attrib(m_ast->attributes, eAstAttribCxxName);

    std::string inner = attrib ? attrib->ident : std::format("impl_{}", get_name());

    const char *privacy = ::get_privacy(m_ast->privacy);

    bool is_const = m_ast->flags & eDeclConst;

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
    m_params.foreach([&](auto param) {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    });

    ref_ast_t *attrib = get_attrib(m_ast->attributes, eAstAttribCxxName);

    std::string inner = attrib ? attrib->ident : std::format("impl_{}", get_name());

    bool is_const = m_ast->flags & eDeclConst;
    bool is_virtual = m_ast->flags & eDeclVirtual;

    const char *virt_str = is_virtual ? "virtual " : "";

    if (m_thunk)
    {
        out.writeln("{}{}({}) {}{{", virt_str, it, params, is_const ? "const " : "");
        out.enter();
        out.writeln("return {}({});", inner, args);
        out.leave();
        out.writeln("}}");
    }
    else
    {
        out.writeln("{}{}({}){};", virt_str, it, params, is_const ? " const" : "");
    }
}

void Method::emit_thunk(out_t& out) const {
    Type *ret = m_return ? m_return->get_type() : new VoidType("void");
    ref_ast_t *attrib = get_attrib(m_ast->attributes, eAstAttribCxxName);
    std::string inner = attrib ? attrib->ident : std::format("impl_{}", get_name());
    auto it = ret->get_cxx_name(inner.c_str());
    std::string params;
    std::string args;
    m_params.foreach([&](auto param) {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    });


    out.writeln("{}({});", it, params);
}

void RecordType::resolve(Sema& sema)
{
    Map<const char*, Method*> methods { vector_len(m_ast->methods), get_global_arena() };

    vec_foreach<ref_ast_t*>(m_ast->methods, [&](auto method) {
        Method *m = new Method(method);
        m->resolve(sema);
        CTASSERTF(!methods.get(method->name), "duplicate method %s", method->name);
        methods.set(method->name, m);

        m_methods.push(m);
    });

    if (m_ast->parent != nullptr)
    {
        m_parent = sema.resolve_type(m_ast->parent);
        CTASSERTF(m_parent != nullptr, "invalid parent type");
    }
}

void RecordType::emit_proto(out_t& out) const
{
    if (get_attrib(m_ast->attributes, eAstAttribExternal) || get_attrib(m_ast->attributes, eAstAttribFacade))
        return;
    out.writeln("{} {};", m_record, get_name());
}

ref_privacy_t RecordType::emit_methods(out_t& out, ref_privacy_t privacy) const
{
    out.writeln("// methods");

    m_methods.foreach([&](auto method) {
        if (privacy != method->get_privacy() && method->get_privacy() != ePrivacyDefault)
        {
            privacy = method->get_privacy();
            out.leave();
            out.writeln("{}:", get_privacy(privacy));
            out.enter();
        }
        method->emit_impl(out);
    });


    out.writeln("// thunks");

    bool emit_private = false;

    m_methods.foreach([&](auto method) {
        if (!method->should_emit_thunk())
        {
            return;
        }

        if (!emit_private)
        {
            emit_private = true;
            out.leave();
            out.writeln("private:");
            out.enter();
        }

        method->emit_thunk(out);
    });

    return ePrivacyPrivate;
}

void RecordType::emit_begin_record(out_t& out, bool write_parent) const
{
    bool is_final = m_ast->flags & eDeclSealed;
    const char *fin = is_final ? " final " : " ";
    if (m_parent && write_parent)
    {
        out.writeln("{} {}{}: public {} {{", m_record, get_name(), fin, m_parent->get_name());
    }
    else
    {
        out.writeln("{} {}{}{{", m_record, get_name(), fin);
    }
    out.enter();

    out.writeln("friend class ctu::TypeInfo<{}>;", get_name());
}

void RecordType::emit_ctors(out_t&) const  {

}

ref_privacy_t RecordType::emit_dtors(out_t& out, ref_privacy_t privacy) const {
    bool is_virtual = m_ast->flags & eDeclVirtual;

    if (!is_virtual)
        return privacy;

    if (privacy != ePrivacyPublic)
    {
        privacy = ePrivacyPublic;
        out.leave();
        out.writeln("{}:", get_privacy(privacy));
        out.enter();
    }

    out.writeln("virtual ~{}() = default;", get_name());

    return privacy;
}


void RecordType::emit_end_record(out_t& out) const
{
    out.leave();
    out.writeln("}};");
}

ref_privacy_t RecordType::emit_fields(out_t& out, const Vector<Field*>& fields, ref_privacy_t privacy) const
{
    out.writeln("// fields");

    fields.foreach([&](auto field) {
        if (privacy != field->get_privacy())
        {
            privacy = field->get_privacy();
            out.leave();
            out.writeln("{}:", get_privacy(privacy));
            out.enter();
        }

        field->emit_field(out);
    });

    return privacy;
}

Class::Class(ref_ast_t *ast)
    : RecordType(ast, eKindClass, "class")
{ }

void Class::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();

    RecordType::resolve(sema);

    Map<const char*, Field*> fields { vector_len(m_ast->tparams), get_global_arena() };

    // TODO

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

    vec_foreach<ref_ast_t*>(m_ast->fields, [&](auto field) {
        Field *f = new Field(field);
        f->resolve(sema);
        CTASSERTF(!fields.get(field->name), "duplicate field %s", field->name);
        fields.set(field->name, f);

        m_fields.push(f);
    });

    if (m_parent != nullptr)
    {
        CTASSERTF(m_parent->get_kind() == eKindClass, "invalid parent type %s", m_parent->get_name());
    }

    finish_resolve();
}

Struct::Struct(ref_ast_t *ast)
    : RecordType(ast, eKindStruct, "struct")
{ }

void Struct::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();
    RecordType::resolve(sema);

    Map<const char*, Field*> fields { vector_len(m_ast->tparams), get_global_arena() };

    vec_foreach<ref_ast_t*>(m_ast->fields, [&](auto field) {
        Field *f = new Field(field);
        f->resolve(sema);
        CTASSERTF(!fields.get(field->name), "duplicate field %s", field->name);
        fields.set(field->name, f);

        m_fields.push(f);
    });

    finish_resolve();
}

Variant::Variant(ref_ast_t *ast)
    : RecordType(ast, eKindVariant, "class")
{ }

void Variant::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();

    RecordType::resolve(sema);

    Map<const char*, Case*> cases { vector_len(m_ast->cases), get_global_arena() };

    vec_foreach<ref_ast_t*>(m_ast->cases, [&](auto field) {
        Case *c = new Case(field);
        c->resolve(sema);
        CTASSERTF(!cases.get(field->name), "duplicate case %s", field->name);
        cases.set(field->name, c);

        m_cases.push(c);
    });

    if (m_parent)
    {
        CTASSERTF(m_parent->get_kind() == eKindTypeInt || m_parent->get_opaque_name() != nullptr, "invalid underlying type %s", m_parent->get_name());
    }

    m_default_case = m_ast->default_case ? cases.get(m_ast->default_case->name) : nullptr;
}

static const char *digit_cxx_name(digit_t digit, sign_t sign)
{
    switch (digit)
    {
    case eDigit8: return (sign == eSignUnsigned) ? "uint8_t" : "int8_t";
    case eDigit16: return (sign == eSignUnsigned) ? "uint16_t" : "int16_t";
    case eDigit32: return (sign == eSignUnsigned) ? "uint32_t" : "int32_t";
    case eDigit64: return (sign == eSignUnsigned) ? "uint64_t" : "int64_t";

    case eDigitFast8: return (sign == eSignUnsigned) ? "uint_fast8_t" : "int_fast8_t";
    case eDigitFast16: return (sign == eSignUnsigned) ? "uint_fast16_t" : "int_fast16_t";
    case eDigitFast32: return (sign == eSignUnsigned) ? "uint_fast32_t" : "int_fast32_t";
    case eDigitFast64: return (sign == eSignUnsigned) ? "uint_fast64_t" : "int_fast64_t";

    case eDigitLeast8: return (sign == eSignUnsigned) ? "uint_least8_t" : "int_least8_t";
    case eDigitLeast16: return (sign == eSignUnsigned) ? "uint_least16_t" : "int_least16_t";
    case eDigitLeast32: return (sign == eSignUnsigned) ? "uint_least32_t" : "int_least32_t";
    case eDigitLeast64: return (sign == eSignUnsigned) ? "uint_least64_t" : "int_least64_t";

    case eDigitChar: return (sign == eSignUnsigned) ? "unsigned char" : "char";
    case eDigitShort: return (sign == eSignUnsigned) ? "unsigned short" : "short";
    case eDigitInt: return (sign == eSignUnsigned) ? "unsigned int" : "int";
    case eDigitLong: return (sign == eSignUnsigned) ? "unsigned long" : "long";
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

void Class::emit_impl(out_t& out) const
{
    if (get_attrib(m_ast->attributes, eAstAttribExternal))
        return;

    emit_begin_record(out);
    auto priv = emit_dtors(out, ePrivacyPrivate);
    priv = emit_fields(out, m_fields, priv);
    priv = emit_methods(out, priv);
    emit_end_record(out);
}

void Struct::emit_impl(out_t& out) const
{
    if (get_attrib(m_ast->attributes, eAstAttribExternal))
        return;

    emit_begin_record(out);
    auto priv = emit_dtors(out, ePrivacyPublic);
    priv = emit_fields(out, m_fields, priv);
    priv = emit_methods(out, priv);
    emit_end_record(out);
}

void Case::emit_impl(out_t& out) const
{
    out.writeln("e{} = {},", get_name(), get_value());
}

static uint32_t type_hash(const char *name)
{
    uint32_t hash = 0xFFFFFFFF;

    while (*name)
    {
        hash ^= *name++;
        for (int i = 0; i < 8; i++)
        {
            hash = (hash >> 1) ^ (0xEDB88320 & -(hash & 1));
        }
    }

    return ~hash;
}

static void get_type_id(ref_ast_t *ast, mpz_t out)
{
    ref_ast_t *attrib = get_attrib(ast->attributes, eAstAttribTypeId);
    if (attrib)
    {
        CTASSERTF(mpz_fits_uint_p(attrib->id), "invalid type id %s, must fit in a uint32_t", mpz_get_str(nullptr, 10, attrib->id));
        mpz_init_set(out, attrib->id);
    }
    else
    {
        mpz_init_set_ui(out, type_hash(ast->name));
    }
}

void Variant::emit_impl(out_t& out) const
{
    bool is_bitflags = get_attrib(m_ast->attributes, eAstAttribBitflags) != nullptr;
    bool is_arithmatic = get_attrib(m_ast->attributes, eAstAttribArithmatic) != nullptr;
    bool is_iterator = get_attrib(m_ast->attributes, eAstAttribIterator) != nullptr;

    std::string ty;
    std::string under;
    out.writeln("namespace impl {{");
    out.enter();
    if (m_parent)
    {
        std::string underlying = m_parent->get_cxx_name(nullptr);
        const char *opaque = m_parent->get_opaque_name();
        if (opaque)
        {
            ty = std::format("{}_underlying_t", get_name());
            under = std::format("impl::{}", ty);
            out.writeln("using {}_underlying_t = std::underlying_type_t<{}>;", get_name(), opaque);
            out.writeln("enum class {} : {}_underlying_t {{", get_name(), get_name());
        }
        else
        {
            ty = underlying;
            under = underlying;
            out.writeln("enum class {} : {} {{", get_name(), underlying);
        }
    }
    else
    {
        under = std::format("impl::{}", get_name());
        out.writeln("enum class {} {{", get_name());
    }

    out.enter();
    m_cases.foreach([&](auto c)
    {
        c->emit_impl(out);
    });
    out.leave();
    out.writeln("}};");
    if (!m_parent)
    {
        ty = std::format("{}_underlying_t", get_name());
        out.writeln("using {}_underlying_t = std::underlying_type_t<{}>;", get_name(), get_name());
    }
    out.writeln("REFLECT_ENUM_COMPARE({}, {})", get_name(), ty);
    if (is_bitflags) out.writeln("REFLECT_ENUM_BITFLAGS({}, {});", get_name(), ty);
    if (is_arithmatic) out.writeln("REFLECT_ENUM_ARITHMATIC({}, {});", get_name(), ty);
    if (is_iterator) out.writeln("REFLECT_ENUM_ITERATOR({}, {});", get_name(), ty);

    out.leave();
    out.writeln("}} // namespace impl");

    if (is_iterator || is_arithmatic)
        CTASSERTF(is_iterator ^ is_arithmatic, "enum %s cannot be both an iterator and arithmatic", get_name());

    emit_begin_record(out, false);
    out.leave();
    out.writeln("public:");
    out.enter();
    out.writeln("using underlying_t = std::underlying_type_t<impl::{}>;", get_name());
    out.writeln("using inner_t = impl::{};", get_name());
    out.nl();
    out.leave();
    out.writeln("private:");
    out.enter();
    out.writeln("inner_t m_value;");
    out.nl();
    out.leave();
    out.writeln("public:");
    out.enter();
    out.writeln("constexpr {}(underlying_t value) : m_value((inner_t)value) {{ }}", get_name());
    out.writeln("constexpr {}(inner_t value) : m_value(value) {{ }}", get_name());
    out.writeln("using enum inner_t;");
    out.nl();
    if (m_default_case)
    {
        out.writeln("static constexpr auto kDefaultCase = e{};", m_default_case->get_name());
        out.writeln("constexpr {}() : m_value(kDefaultCase) {{ }}", get_name());
    }
    else
    {
        out.writeln("constexpr {}() = delete;", get_name());
    }

    if (is_iterator)
    {
        out.nl();
        out.writeln("static constexpr auto kBegin = (inner_t)((underlying_t)0);");
        out.writeln("static constexpr auto kEnd = (inner_t)(~(underlying_t)0);");
        out.nl();
        out.writeln("class Iterator {{");
        out.enter();
        out.writeln("inner_t m_value;");
        out.leave();
        out.writeln("public:");
        out.enter();
        out.writeln("constexpr Iterator(inner_t value) : m_value(value) {{ }}");
        out.writeln("constexpr Iterator& operator++() {{ m_value = (inner_t)((underlying_t)m_value + 1); return *this; }}");
        out.writeln("constexpr const Iterator operator++(int) {{ Iterator it = *this; ++(*this); return it; }}");
        out.writeln("constexpr bool operator==(const Iterator& other) const {{ return m_value == other.m_value; }}");
        out.writeln("constexpr bool operator!=(const Iterator& other) const {{ return m_value != other.m_value; }}");
        out.writeln("constexpr {} operator*() const {{ return m_value; }}", get_name());
        out.leave();
        out.writeln("}};");
        out.nl();
        out.writeln("class Range {{");
        out.enter();
        out.writeln("inner_t m_begin;");
        out.writeln("inner_t m_end;");
        out.leave();
        out.writeln("public:");
        out.enter();
        out.writeln("constexpr Range(inner_t begin, inner_t end) : m_begin(begin), m_end(end) {{ }}");
        out.writeln("constexpr Iterator begin() const {{ return Iterator(m_begin); }}");
        out.writeln("constexpr Iterator end() const {{ return Iterator(m_end); }}");
        out.leave();
        out.writeln("}};");
        out.nl();

        out.writeln("static constexpr Range range(inner_t begin, inner_t end) {{ return Range(begin, end); }}");
    }

    out.writeln("constexpr operator inner_t() const {{ return m_value; }}");

    // out.writeln("constexpr {}(const {}& other) = default;", get_name(), get_name());
    // out.writeln("constexpr {}& operator=(const {}& other) = default;", get_name(), get_name());

    // out.writeln("constexpr {}(const {}&& other) = default;", get_name(), get_name());
    // out.writeln("constexpr {}& operator=(const {}&& other) = default;", get_name(), get_name());

    out.writeln("constexpr underlying_t as_integral() const {{ return (underlying_t)m_value; }}");
    out.writeln("constexpr inner_t as_enum() const {{ return m_value; }}");

    out.nl();
    out.writeln("constexpr bool operator==(inner_t other) const {{ return m_value == other; }}");
    out.writeln("constexpr bool operator!=(inner_t other) const {{ return m_value != other; }}");

    if (!is_bitflags && !is_arithmatic)
    {
        out.nl();
        out.writeln("constexpr bool is_valid() const {{");
        out.enter();
        out.writeln("switch (m_value) {{");
        m_cases.foreach([&](auto c)
        {
            out.writeln("case e{}:", c->get_name());
        });
        out.enter();
        out.writeln("return true;");
        out.leave();
        out.writeln("default: return false;");
        out.writeln("}}");
        out.leave();
        out.writeln("}};");
    }

    if (is_bitflags)
    {
        std::string flags;
        m_cases.foreach([&](auto c)
        {
            if (!flags.empty())
                flags += " | ";
            flags += std::format("e{}", c->get_name());
        });
        out.writeln("static constexpr {} none() {{ return {}((inner_t)0); }};", get_name(), get_name());
        out.writeln("static constexpr {} mask() {{ return {}({}); }};", get_name(), get_name(), flags);
        // emit bitwise operators
        out.nl();
        out.writeln("constexpr {} operator~() const {{ return ~m_value; }}", get_name());
        out.writeln("constexpr {} operator|(const {}& other) const {{ return m_value | other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {} operator&(const {}& other) const {{ return m_value & other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {} operator^(const {}& other) const {{ return m_value ^ other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator|=(const {}& other) {{ m_value = m_value | other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator&=(const {}& other) {{ m_value = m_value & other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator^=(const {}& other) {{ m_value = m_value ^ other.m_value; return *this; }}", get_name(), get_name());

        out.writeln("constexpr bool test(inner_t other) const {{ return (m_value & other) != none(); }}");
        out.writeln("constexpr bool any(inner_t other) const {{ return (m_value & other) != none(); }}");
        out.writeln("constexpr bool all(inner_t other) const {{ return (m_value & other) == other; }}");
        out.writeln("constexpr bool none(inner_t other) const {{ return (m_value & other) == none(); }}");
        out.writeln("constexpr {}& set(inner_t other) {{ m_value = m_value | other; return *this; }}", get_name());
        out.writeln("constexpr {}& reset(inner_t other) {{ m_value = m_value & ~other; return *this; }}", get_name());
        out.writeln("constexpr {}& flip(inner_t other) {{ m_value = m_value ^ other; return *this; }}", get_name());

        // is_valid is defined as no invalid flags set
        out.writeln("constexpr bool is_valid() const {{ return (m_value & ~mask()) == none(); }}");
    }

    if (is_arithmatic)
    {
        // implement arithmatic operators
        out.nl();
        out.writeln("constexpr {} operator+(const {}& other) const {{ return m_value + other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {} operator-(const {}& other) const {{ return m_value - other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {} operator*(const {}& other) const {{ return m_value * other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {} operator/(const {}& other) const {{ return m_value / other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {} operator%(const {}& other) const {{ return m_value % other.m_value; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator+=(const {}& other) {{ m_value = m_value + other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator-=(const {}& other) {{ m_value = m_value - other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator*=(const {}& other) {{ m_value = m_value * other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator/=(const {}& other) {{ m_value = m_value / other.m_value; return *this; }}", get_name(), get_name());
        out.writeln("constexpr {}& operator%=(const {}& other) {{ m_value = m_value % other.m_value; return *this; }}", get_name(), get_name());

        // is_valid is not defined for arithmatic types
    }

    emit_methods(out, ePrivacyPublic);

    emit_end_record(out);
    out.nl();
    out.writeln("static_assert(sizeof({}) == sizeof({}::underlying_t), \"{} size mismatch\");", get_name(), get_name(), get_name());
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

static void emit_record_fields(out_t& out, const Vector<Field*>& fields)
{
    out.writeln("static constexpr field_t kFields[{}] = {{", fields.size());
        out.enter();
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields.get(i);
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

static void emit_record_visit(out_t& out, const std::string& id, const Vector<Field*>& fields)
{
    out.writeln("constexpr auto visit_field({}& object, const field_t& field, auto&& fn) const {{", id);
    out.enter();
        out.writeln("switch (field.index) {{");
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields.get(i);
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
            auto f = fields.get(i);
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
    out.writeln("return TypeInfo<{}>{{}};", id);
    out.leave();
    out.writeln("}}");
}

static void emit_info_header(out_t& out, const std::string& id)
{
    out.writeln("template<> class TypeInfo<{}> : public TypeInfoBase {{", id);
    out.writeln("public:");
}

static std::string get_decl_name(ref_ast_t *ast, Sema& sema, const char *name)
{
    ref_ast_t *external = get_attrib(ast->attributes, eAstAttribExternal);
    if (external)
    {
        return name;
    }
    else
    {
        return std::format("{}::{}", sema.get_namespace(), name);
    }
}

void Struct::emit_reflection(Sema& sema, out_t& out) const
{
    if (ref_ast_t *noreflect = get_attrib(m_ast->attributes, eAstAttribNoReflect); noreflect)
    {
        return;
    }

    auto id = get_decl_name(m_ast, sema, get_name());

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

    auto id = get_decl_name(m_ast, sema, get_name());
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
                auto m = m_methods.get(i);
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

size_t Variant::max_tostring() const {
    if (get_attrib(m_ast->attributes, eAstAttribBitflags))
        return max_tostring_bitflags();

    size_t max = 0;

    m_cases.foreach([&](auto c)
    {
        size_t len = strlen(c->get_name());
        if (len > max)
            max = len;
    });

    return max + 1;
}

size_t Variant::max_tostring_bitflags() const {
    // sum all the cases + 1 for each comma
    size_t max = 0;

    m_cases.foreach([&](auto c)
    {
        size_t len = strlen(c->get_name());
        max += len + 1;
    });

    return max;
}

void Variant::emit_reflection(Sema& sema, out_t& out) const
{
    if (ref_ast_t *noreflect = get_attrib(m_ast->attributes, eAstAttribNoReflect); noreflect)
    {
        return;
    }

    auto id = get_decl_name(m_ast, sema, get_name());

    mpz_t typeid_value;
    get_type_id(m_ast, typeid_value);
    bool is_bitflags = get_attrib(m_ast->attributes, eAstAttribBitflags) != nullptr;

    size_t max_tostring_length = max_tostring();

    emit_info_header(out, id);
        out.enter();
        out.writeln("using type_t = {};", id);
        out.writeln("using underlying_t = {}::underlying_t;", id);
        out.writeln("using case_t = ctu::EnumCase<{}>;", id);
        out.nl();
        out.writeln("static constexpr size_t kMaxLength = {};", max_tostring_length);
        out.writeln("using string_t = SmallString<kMaxLength>;");
        out.nl();
        emit_name_info(out, id, m_ast);
        if (m_parent)
            out.writeln("static constexpr TypeInfo<underlying_t> kUnderlying{{}};");
        else
            out.writeln("static constexpr TypeInfo<void> kUnderlying{{}};");

        out.writeln("static constexpr bool kHasDefault = {};", m_default_case != nullptr);
        if (m_default_case)
            out.writeln("static constexpr {} kDefaultCase = {}::e{};", id, id, m_default_case->get_name());

        out.nl();
        out.writeln("static constexpr case_t kCases[{}] = {{", m_cases.size());
            out.enter();
            m_cases.foreach([&](auto c) {
                out.writeln("case_t {{ impl::objname(\"e{}\"), {}::e{} }},", c->get_name(), id, c->get_name());
            });
            out.leave();
        out.writeln("}};");
        out.nl();
        emit_ctor(out);
        out.nl();

        out.nl();
        out.writeln("constexpr string_t to_string(type_t value, [[maybe_unused]] int base = 10) const {{");
        out.enter();
        if (is_bitflags)
        {
            out.writeln("string_t result;");
            out.writeln("bool first = true;");
            out.writeln("for (auto option : kCases) {{");
            out.enter();
            out.writeln("if ((value & option.value) == option.value) {{");
            out.enter();
            out.writeln("if (!first) result += \", \";");
            out.writeln("result += option.name;");
            out.writeln("first = false;");
            out.leave();
            out.writeln("}}");
            out.leave();
            out.writeln("}}");
            out.writeln("return result;");
        }
        else
        {
            out.writeln("for (auto option : kCases) {{");
            out.enter();
            out.writeln("if (option.value == value) return option.name;");
            out.leave();
            out.writeln("}}");
            out.writeln("return string_t(value.as_integral(), base);");
        }
        out.leave();
        out.writeln("}};");
    out.leave();
    out.writeln("}};");

    out.nl();
    emit_reflect_hook(out, id);
    out.nl();
}
