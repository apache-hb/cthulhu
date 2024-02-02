#include "ref/sema.hpp"
#include "core/macros.h"
#include "cthulhu/events/events.h"
#include "memory/memory.h"
#include "ref/ast.h"
#include "ref/eval.h"
#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"
#include <climits>

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

    decls.set("memory", new MemoryType("memory"));
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
    // TODO: pick out api
    // if (mod->api) m_api = mod->api;

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
    case eAstAlias: {
        TypeAlias *alias = new TypeAlias(ast);
        add_decl(name, alias);
        return alias;
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
    case eAstIdent: {
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
    case eAstPointer: {
        Type *type = resolve_type(ast->ptr);
        if (type == nullptr)
        {
            report(&kEvent_InvalidType, ast->node, "invalid pointer type");
            return nullptr;
        }

        return new PointerType(ast->node, type);
    }
    case eAstReference: {
        Type *type = resolve_type(ast->ptr);
        if (type == nullptr)
        {
            report(&kEvent_InvalidType, ast->node, "invalid reference type");
            return nullptr;
        }

        if (type->get_kind() == eKindReference)
        {
            report(&kEvent_InvalidType, ast->node, "cannot make a reference to a reference");
            return nullptr;
        }

        if (type->get_kind() == eKindTypePointer)
        {
            report(&kEvent_InvalidType, ast->node, "cannot make a reference to memory");
            return nullptr;
        }

        if (type->get_kind() == eKindTypeVoid)
        {
            report(&kEvent_InvalidType, ast->node, "cannot make a reference to void");
            return nullptr;
        }

        if (type->get_kind() == eKindTypeMemory)
        {
            report(&kEvent_InvalidType, ast->node, "cannot make a reference to memory");
            return nullptr;
        }

        return new ReferenceType(ast->node, type);
    }
    case eAstOpaque: {
        return new OpaqueType(ast->node, ast->ident);
    }
    case eAstConst: {
        return new ConstType(ast->node, resolve_type(ast->type));
    }

    default: {
        report(&kEvent_InvalidType, ast->node, "invalid type");
        return nullptr;
    }
    }
}

void Sema::emit_all(io_t *header, const char *file)
{
    // header preamble
    cxx_emit_t *h = cxx_emit_new(m_arena);
    cxx_writeln(h, "#pragma once");
    cxx_writeln(h, "// Generated from '%s'", file);
    cxx_writeln(h, "// Dont edit this file, it will be overwritten on the next build");
    cxx_nl(h);
    cxx_writeln(h, "#include \"reflect/reflect.h\"");
    cxx_nl(h);

    imports.foreach([&](auto fd) {
        if (fd[0] == '<')
        {
            cxx_writeln(h, "#include %s", fd);
        }
        else
        {
            cxx_writeln(h, "#include \"%s\"", fd);
        }
    });

    cxx_writeln(h, "namespace %s {", m_namespace);
    cxx_enter(h);

    cxx_writeln(h, "// prototypes");

    m_decls.foreach([&](auto, auto decl) {
        decl->emit_proto(h);
    });

    cxx_nl(h);
    cxx_writeln(h, "// implementation");

    DeclDepends depends { { 64, get_global_arena() } };

    m_decls.foreach([&](auto, auto decl) {
        decl->get_deps(depends);
        depends.add(decl);
    });

    depends.m_depends.foreach([&](auto decl) {
        CTASSERT(decl != nullptr);
        decl->emit_impl(h);
    });

    cxx_leave(h);
    cxx_writeln(h, "} // namespace %s", m_namespace);

    cxx_nl(h);
    cxx_writeln(h, "namespace ctu {");
    cxx_enter(h);
    cxx_writeln(h, "// reflection");

    depends.m_depends.foreach([&](auto decl) {
        CTASSERT(decl != nullptr);
        decl->emit_reflection(*this, h);
    });


    cxx_leave(h);
    cxx_writeln(h, "} // namespace ctu");

    cxx_dump(h, header);
}

void Field::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();

    resolve_type(sema);
}

Field::Field(ref_ast_t *ast)
    : TreeBackedDecl(ast, eKindField)
{ }

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

static const char *get_attrib_string(vector_t *attribs, ref_attrib_tag_t tag)
{
    CTASSERT(attribs != nullptr);

    size_t len = vector_len(attribs);
    for (size_t i = 0; i < len; i++)
    {
        ref_ast_t *attrib = (ref_ast_t*)vector_get(attribs, i);
        if (attrib->kind != eAstAttribString)
            continue;

        if (attrib->attrib == tag)
            return attrib->ident;
    }

    return nullptr;
}

static const char *get_doc(vector_t *attribs, const char *key)
{
    ref_ast_t *docs = get_attrib(attribs, eAstAttribDocs);
    if (docs == nullptr)
        return nullptr;

    const char *doc = (const char*)map_get(docs->docs, key);
    if (doc == nullptr)
        return nullptr;

    return doc;
}

void Decl::find_layout(const ref_ast_t *ast, logger_t *logger)
{
    vector_t *attribs = ast->attributes;
    ref_ast_t *layout_ast = NULL;
    ref_attrib_tag_t layout_tag = get_layout();

    size_t len = vector_len(attribs);
    for (size_t i = 0; i < len; i++)
    {
        ref_ast_t *attrib = (ref_ast_t*)vector_get(attribs, i);
        CTASSERTF(attrib != NULL, "invalid attribute");

        switch (attrib->attrib)
        {
        case eAttribLayoutSystem:
        case eAttribLayoutStable:
        case eAttribLayoutPacked:
        case eAttribLayoutAny:
        case eAttribLayoutConstBuffer:
            if (layout_ast != NULL)
            {
                msg_notify(logger, &kEvent_DuplicateAttribute, attrib->node, "duplicate layout attribute");
                return;
            }
            layout_ast = attrib;
            layout_tag = attrib->attrib;
            break;

        case eAttribSerialize:
            m_serialize = true;
            layout_tag = eAttribLayoutStable;
            break;
        case eAttribChecksum:
            m_checksum = true;
            layout_tag = eAttribLayoutStable;
            break;
        case eAttribVertexInput:
            m_vbo_layout = true;
            layout_tag = eAttribLayoutStable;
            break;

        default: break;
        }
    }
}

const char *TreeBackedDecl::get_repr() const
{
    if (const char *repr = get_attrib_string(m_ast->attributes, eAttribFormat))
        return repr;

    return get_name();
}

const char *Case::get_repr() const
{
    if (const char *repr = get_attrib_string(m_ast->attributes, eAttribFormat))
        return repr;

    return refl_fmt("e%s", get_name());
}

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
    : TreeBackedDecl(ast, eKindCase)
{ }

void Case::resolve(Sema& sema)
{
    if (is_resolved()) return;
    finish_resolve();

    if (m_ast->value != nullptr)
    {
        m_eval = eval_expr(digit_value, sema.get_logger(), m_ast->value);
    }
}

const char* Case::get_case_value() const {
    if (m_ast->value == nullptr)
        return nullptr;

    if (m_ast->value->kind == eAstOpaque)
        return m_ast->value->ident;

    CTASSERTF(m_eval == eEvalOk, "could not compute case value for %s", get_name());

    return mpz_get_str(nullptr, 10, digit_value);
}

bool Case::is_opaque_case() const {
    return m_eval == eEvalOpaque;
}

bool Case::is_blank_case() const {
    return m_ast->value == nullptr;
}

bool Case::get_integer(mpz_t out) const {
    if (m_eval == eEvalOk)
    {
        mpz_init_set(out, digit_value);
        return true;
    }

    return false;
}

static bool has_attrib_tag(vector_t *attribs, ref_attrib_tag_t tag)
{
    ref_ast_t *attrib = get_attrib(attribs, eAstAttribTag);
    if (!attrib) return false;

    return attrib->attrib == tag;
}

void Method::resolve(Sema& sema) {
    if (is_resolved()) return;
    finish_resolve();

    if (m_ast->return_type != nullptr)
        m_return = sema.resolve_type(m_ast->return_type);

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

    const char *cxxname = get_attrib_string(m_ast->attributes, eAttribCxxName);
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

void Method::emit_impl(cxx_emit_t *out) const {
    Type *ret = m_return ? m_return->get_type() : new VoidType("void");
    auto it = ret->get_cxx_name(get_name());
    String params;
    String args;
    m_params.foreach([&](auto param) {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    });

    const char *cxxname = get_attrib_string(m_ast->attributes, eAttribCxxName);

    const char* inner = cxxname ? cxxname : refl_fmt("impl_%s", get_name());

    const char *privacy = ::get_privacy(m_ast->privacy);

    bool is_const = m_ast->flags & eDeclConst;

    if (m_thunk)
    {
        cxx_writeln(out, "%s: %s(%.*s)%s{", privacy, it, (int)params.size(), params.c_str(), is_const ? " const" : "");
        cxx_enter(out);
        cxx_writeln(out, "return %s(%.*s);", inner, (int)args.size(), args.c_str());
        cxx_close_brace(out);
    }
    else
    {
        cxx_writeln(out, "%s: %s(%.*s)%s;", privacy, it, (int)params.size(), params.c_str(), is_const ? " const" : "");
    }
}

void Method::emit_method(cxx_emit_t *out, const RecordType& parent) const {
    Type *ret = m_return ? m_return : new VoidType("void");
    auto it = ret->get_cxx_name(get_name());
    String params;
    String args;
    m_params.foreach([&](auto param) {
        if (!args.empty())
            args += ", ";
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
        args += param->get_name();
    });

    const char *cxxname = get_attrib_string(m_ast->attributes, eAttribCxxName);

    const char* inner = cxxname ? cxxname : refl_fmt("impl_%s", get_name());

    bool is_const = m_ast->flags & eDeclConst;
    bool is_virtual = m_ast->flags & eDeclVirtual;

    if (is_virtual && !parent.is_virtual())
    {
        NEVER("virtual method %s on non-virtual class %s", get_name(), parent.get_name());
    }

    const char *virt_str = is_virtual ? "virtual " : "";

    if (m_thunk)
    {
        cxx_writeln(out, "%s%s(%.*s)%s", virt_str, it, (int)params.size(), params.c_str(), is_const ? " const" : "");
        cxx_enter(out);
        cxx_writeln(out, "return %s(%.*s);", inner, (int)args.size(), args.c_str());
        cxx_close_brace(out);
    }
    else
    {
        cxx_writeln(out, "%s%s(%.*s)%s;", virt_str, it, (int)params.size(), params.c_str(), is_const ? " const" : "");
    }
}

void Method::emit_thunk(cxx_emit_t *out) const {
    Type *ret = m_return ? m_return->get_type() : new VoidType("void");
    const char *cxxname = get_attrib_string(m_ast->attributes, eAttribCxxName);
    const char* inner = cxxname ? cxxname : refl_fmt("impl_%s", get_name());
    auto it = ret->get_cxx_name(inner);
    String params;
    m_params.foreach([&](auto param) {
        if (!params.empty())
            params += ", ";
        params += param->get_type()->get_cxx_name(param->get_name());
    });


    cxx_writeln(out, "%s(%.*s);", it, (int)params.size(), params.c_str());
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

static bool type_is_external(ref_ast_t *ast)
{
    return has_attrib_tag(ast->attributes, eAttribExternal);
}

static bool type_is_facade(ref_ast_t *ast)
{
    return has_attrib_tag(ast->attributes, eAttribFacade);
}

// internal types have no reflection data
static bool type_is_internal(ref_ast_t *ast)
{
    return has_attrib_tag(ast->attributes, eAttribInternal);
}

bool RecordType::is_stable_layout() const
{
    return has_attrib_tag(m_ast->attributes, eAttribLayoutStable);
}

void RecordType::emit_proto(cxx_emit_t *out) const
{
    if (type_is_external(m_ast))
        return;
    cxx_writeln(out, "%s %s;", m_record, get_name());
}

ref_privacy_t RecordType::emit_methods(cxx_emit_t *out, ref_privacy_t privacy) const
{
    cxx_writeln(out, "// methods");

    m_methods.foreach([&](auto method) {
        if (privacy != method->get_privacy() && method->get_privacy() != ePrivacyDefault)
        {
            privacy = method->get_privacy();
            cxx_privacy(out, get_privacy(privacy));
        }
        method->emit_method(out, *this);
    });


    cxx_writeln(out, "// thunks");

    bool emit_private = false;

    m_methods.foreach([&](auto method) {
        if (!method->should_emit_thunk())
        {
            return;
        }

        if (!emit_private)
        {
            emit_private = true;
            cxx_privacy(out, "private");
        }

        method->emit_thunk(out);
    });

    return ePrivacyPrivate;
}

void RecordType::emit_begin_record(cxx_emit_t *out, bool write_parent) const
{
    const char *fin = is_final() ? " final " : " ";
    if (m_parent && write_parent)
    {
        cxx_writeln(out, "%s %s%s: public %s {", m_record, get_name(), fin, m_parent->get_name());
    }
    else
    {
        cxx_writeln(out, "%s %s%s{", m_record, get_name(), fin);
    }
    cxx_enter(out);

    cxx_writeln(out, "friend class ctu::TypeInfo<%s>;", get_name());
}

void RecordType::emit_ctors(cxx_emit_t*) const  {

}

ref_privacy_t RecordType::emit_dtors(cxx_emit_t *out, ref_privacy_t privacy) const {
    if (!is_virtual())
        return privacy;

    if (privacy != ePrivacyPublic)
    {
        privacy = ePrivacyPublic;
        cxx_privacy(out, get_privacy(privacy));
    }

    cxx_writeln(out, "virtual ~%s() = default;", get_name());

    return privacy;
}


void RecordType::emit_end_record(cxx_emit_t *out) const
{
    cxx_leave(out);
    cxx_writeln(out, "};");
}

ref_privacy_t RecordType::emit_fields(cxx_emit_t *out, const Vector<Field*>& fields, ref_privacy_t privacy) const
{
    cxx_writeln(out, "// fields");

    fields.foreach([&](auto field) {
        if (privacy != field->get_privacy())
        {
            privacy = field->get_privacy();
            cxx_privacy(out, get_privacy(privacy));
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
    : RecordType(ast, eKindClass, "struct")
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

static size_t digit_sizeof(digit_t digit)
{
    switch (digit)
    {
    case eDigit8: return sizeof(uint8_t);
    case eDigit16: return sizeof(uint16_t);
    case eDigit32: return sizeof(uint32_t);
    case eDigit64: return sizeof(uint64_t);

    case eDigitFast8: return sizeof(uint_fast8_t);
    case eDigitFast16: return sizeof(uint_fast16_t);
    case eDigitFast32: return sizeof(uint_fast32_t);
    case eDigitFast64: return sizeof(uint_fast64_t);

    case eDigitLeast8: return sizeof(uint_least8_t);
    case eDigitLeast16: return sizeof(uint_least16_t);
    case eDigitLeast32: return sizeof(uint_least32_t);
    case eDigitLeast64: return sizeof(uint_least64_t);

    case eDigitChar: return sizeof(char);
    case eDigitShort: return sizeof(short);
    case eDigitInt: return sizeof(int);
    case eDigitLong: return sizeof(long);
    case eDigitSize: return sizeof(size_t);
    case eDigitPtr: return sizeof(intptr_t);

    default: NEVER("invalid digit %d", digit);
    }
}

static size_t digit_alignof(digit_t digit)
{
    switch (digit)
    {
    case eDigit8: return alignof(uint8_t);
    case eDigit16: return alignof(uint16_t);
    case eDigit32: return alignof(uint32_t);
    case eDigit64: return alignof(uint64_t);

    case eDigitFast8: return alignof(uint_fast8_t);
    case eDigitFast16: return alignof(uint_fast16_t);
    case eDigitFast32: return alignof(uint_fast32_t);
    case eDigitFast64: return alignof(uint_fast64_t);

    case eDigitLeast8: return alignof(uint_least8_t);
    case eDigitLeast16: return alignof(uint_least16_t);
    case eDigitLeast32: return alignof(uint_least32_t);
    case eDigitLeast64: return alignof(uint_least64_t);

    case eDigitChar: return alignof(char);
    case eDigitShort: return alignof(short);
    case eDigitInt: return alignof(int);
    case eDigitLong: return alignof(long);
    case eDigitSize: return alignof(size_t);
    case eDigitPtr: return alignof(intptr_t);

    default: NEVER("invalid digit %d", digit);
    }
}

IntType::IntType(const char *name, digit_t digit, sign_t sign)
    : Type(node_builtin(), eKindTypeInt, name, digit_sizeof(digit), digit_alignof(digit))
    , m_digit(digit)
    , m_sign(sign)
{ }

const char* IntType::get_cxx_name(const char *name) const
{
    const char *type = digit_cxx_name(m_digit, m_sign);
    return (name == nullptr) ? type : refl_fmt("%s %s", type, name);
}

void Field::emit_impl(cxx_emit_t *out) const
{
    const char *privacy = ::get_privacy(m_ast->privacy);
    auto it = get_type()->get_cxx_name(get_name());
    cxx_writeln(out, "%s: %s;", privacy, it);
}

void Field::emit_field(cxx_emit_t *out) const
{
    auto it = get_type()->get_cxx_name(get_name());
    cxx_writeln(out, "%s;", it);
}

bool Field::is_transient() const
{
    return has_attrib_tag(m_ast->attributes, eAttribTransient);
}

void Class::emit_impl(cxx_emit_t *out) const
{
    if (type_is_external(m_ast))
        return;

    emit_begin_record(out);
    auto priv = emit_dtors(out, ePrivacyPrivate);
    priv = emit_fields(out, m_fields, priv);
    priv = emit_methods(out, priv);
    emit_end_record(out);
}

void Struct::emit_impl(cxx_emit_t *out) const
{
    if (type_is_external(m_ast))
        return;

    emit_begin_record(out);
    auto priv = emit_dtors(out, ePrivacyPublic);
    priv = emit_fields(out, m_fields, priv);
    priv = emit_methods(out, priv);
    emit_end_record(out);
}

void Case::emit_impl(cxx_emit_t *out) const
{
    cxx_writeln(out, "e%s = %s,", get_name(), get_case_value());
}

static uint32_t type_hash(const char *name)
{
    uint32_t hash = 0xFFFFFFFF;

    while (*name)
    {
        hash ^= *name++;
        hash *= 16777619;
    }

    return ~hash;
}

static void get_type_id(Sema& sema, const ref_ast_t *ast, mpz_t out)
{
    ref_ast_t *attrib = get_attrib(ast->attributes, eAstAttribTypeId);
    if (attrib)
    {
        if (eval_expr(out, sema.get_logger(), attrib->expr) != eEvalOk)
        {
            sema.report(&kEvent_InvalidType, attrib->node, "could not evaluate typeid to an integer");
            return;
        }

        if (!mpz_fits_uint_p(out))
        {
            sema.report(&kEvent_IntegerOverflow, attrib->node, "typeid must fit in a uint32_t");
        }
    }
    else
    {
        mpz_init_set_ui(out, type_hash(ast->name));
    }
}

void Variant::emit_default_is_valid(cxx_emit_t *out) const
{
    size_t len = m_cases.size();
    typevec_t *found = typevec_new(sizeof(mpz_t), len, get_global_arena());

    auto has_value = [&](mpz_t value) -> bool {
        for (size_t i = 0; i < typevec_len(found); i++)
        {
            mpz_t *it = (mpz_t*)typevec_offset(found, i);
            if (mpz_cmp(*it, value) == 0)
                return true;
        }
        return false;
    };

    cxx_nl(out);
    cxx_writeln(out, "constexpr bool is_valid() const {");
    cxx_enter(out);
    cxx_writeln(out, "switch (m_value) {");
    m_cases.foreach([&](Case *c)
    {
        mpz_t id;
        if (c->get_integer(id))
        {
            // if we found a duplicate, dont emit this case
            if (has_value(id))
            {
                cxx_writeln(out, "// duplicate case %s", c->get_name());
                return;
            }

            typevec_push(found, &id);
        }

        cxx_writeln(out, "case e%s:", c->get_name());
    });
    cxx_enter(out);
    cxx_writeln(out, "return true;");
    cxx_leave(out);
    cxx_writeln(out, "default: return false;");
    cxx_writeln(out, "}");
    cxx_leave(out);
    cxx_writeln(out, "};");
}

void Variant::emit_impl(cxx_emit_t *out) const
{
    bool is_ordered = has_attrib_tag(m_ast->attributes, eAttribOrdered);
    bool is_bitflags = has_attrib_tag(m_ast->attributes, eAttribBitflags);
    bool is_arithmatic = has_attrib_tag(m_ast->attributes, eAttribArithmatic);
    bool is_iterator = has_attrib_tag(m_ast->attributes, eAttribIterator);
    bool is_lookup = has_attrib_tag(m_ast->attributes, eAttribLookupKey);

    const char *ty = nullptr;
    bool is_external = type_is_external(m_ast);
    bool is_facade = type_is_facade(m_ast);
    if (is_external || is_facade)
    {
        CTASSERTF(m_parent != nullptr, "enum %s must have a parent because it is not implemented internally", get_name());
        CTASSERTF(!(is_facade && is_external), "enum %s cannot be both a facade and external", get_name());
    }

    cxx_writeln(out, "namespace impl {");
    cxx_enter(out);
    if (m_parent)
    {
        if (const char *opaque = m_parent->get_opaque_name())
        {
            ty = refl_fmt("%s_underlying_t", get_name());
            cxx_writeln(out, "using %s_underlying_t = std::underlying_type_t<%s>;", get_name(), opaque);
            cxx_writeln(out, "enum class %s : %s_underlying_t {", get_name(), get_name());
        }
        else
        {
            const char *underlying = m_parent->get_cxx_name(nullptr);
            ty = underlying;
            cxx_writeln(out, "enum class %s : %s {", get_name(), underlying);
        }
    }
    else
    {
        cxx_writeln(out, "enum class %s {", get_name());
    }

    mpz_t lowest;
    mpz_init_set_ui(lowest, INT_MAX);
    mpz_t highest;
    mpz_init_set_si(highest, INT_MIN);
    mpz_t current;
    mpz_init_set_si(current, -1);

    bool has_opaque_cases = false;

    cxx_enter(out);
    m_cases.foreach([&](auto c)
    {
        if (c->is_opaque_case())
        {
            CTASSERTF(!is_lookup, "variant %s cannot have opaque cases and be a lookup key", get_name());

            has_opaque_cases = true;
            cxx_writeln(out, "e%s = %s,", c->get_name(), c->get_case_value());
        }
        else if (c->is_blank_case())
        {
            CTASSERTF(!has_opaque_cases, "cannot generate case values in a variant %s with opaque cases", get_name());

            mpz_add_ui(current, current, 1);
            cxx_writeln(out, "e%s = %s,", c->get_name(), mpz_get_str(nullptr, 10, current));

            if (mpz_cmp(current, lowest) < 0)
                mpz_set(lowest, current);
            if (mpz_cmp(current, highest) > 0)
                mpz_set(highest, current);
        }
        else if (mpz_t value; c->get_integer(value))
        {
            if (mpz_cmp(value, lowest) < 0)
                mpz_set(lowest, value);
            if (mpz_cmp(value, highest) > 0)
                mpz_set(highest, value);

            mpz_set(current, value);
            cxx_writeln(out, "e%s = %s,", c->get_name(), c->get_case_value());
        }
    });
    cxx_leave(out);
    cxx_writeln(out, "};");
    if (!m_parent)
    {
        ty = refl_fmt("%s_underlying_t", get_name());
        cxx_writeln(out, "using %s_underlying_t = std::underlying_type_t<%s>;", get_name(), get_name());
    }
    if (is_arithmatic || is_iterator || is_ordered) cxx_writeln(out, "REFLECT_ENUM_COMPARE(%s, %s)", get_name(), ty);
    if (is_bitflags) cxx_writeln(out, "REFLECT_ENUM_BITFLAGS(%s, %s);", get_name(), ty);
    if (is_arithmatic) cxx_writeln(out, "REFLECT_ENUM_ARITHMATIC(%s, %s);", get_name(), ty);
    if (is_iterator) cxx_writeln(out, "REFLECT_ENUM_ITERATOR(%s, %s);", get_name(), ty);

    cxx_leave(out);
    cxx_writeln(out, "} // namespace impl");

    if (is_iterator || is_arithmatic)
        CTASSERTF(is_iterator ^ is_arithmatic, "enum %s cannot be both an iterator and arithmatic", get_name());

    emit_begin_record(out, false);
    cxx_privacy(out, "public");
    // underlying is the true integral type of the enum
    cxx_writeln(out, "using underlying_t = std::underlying_type_t<impl::%s>;", get_name());

    bool facade_type_clash = false;

    // wrapper is our scoped enum type
    cxx_writeln(out, "using wrapper_t = impl::%s;", get_name());
    if (is_facade)
    {
        if (const char *opaque = m_parent->get_opaque_name())
        {
            // facade is the type that the user specified if this is a facade type
            // this is distinct from underlying for C style enums
            cxx_writeln(out, "using facade_t = %s;", opaque);
        }
        else
        {
            cxx_writeln(out, "using facade_t = %s;", m_parent->get_cxx_name(nullptr));

            facade_type_clash = true;
        }
    }
    cxx_writeln(out, "using Underlying = underlying_t;");
    cxx_writeln(out, "using Inner = wrapper_t;");
    if (is_facade)
    {
        cxx_writeln(out, "using Facade = facade_t;");
    }
    cxx_nl(out);
    cxx_privacy(out, "private");
    cxx_writeln(out, "wrapper_t m_value;");
    cxx_nl(out);
    cxx_privacy(out, "public");
    cxx_writeln(out, "constexpr %s(underlying_t value) : m_value((wrapper_t)value) { }", get_name());
    cxx_writeln(out, "constexpr %s(wrapper_t value) : m_value(value) { }", get_name());
    if (is_facade && !facade_type_clash)
    {
        cxx_writeln(out, "constexpr %s(facade_t value) : m_value((wrapper_t)value) { }", get_name());
    }
    cxx_writeln(out, "using enum impl::%s;", get_name());
    cxx_nl(out);
    if (m_default_case)
    {
        cxx_writeln(out, "static constexpr auto kDefaultCase = e%s;", m_default_case->get_name());
        cxx_writeln(out, "constexpr %s() : m_value(kDefaultCase) { }", get_name());
    }
    else
    {
        cxx_writeln(out, "constexpr %s() = delete;", get_name());
    }

    if (is_iterator)
    {
        cxx_nl(out);
        cxx_writeln(out, "static constexpr auto kBegin = (wrapper_t)((underlying_t)0);");
        cxx_writeln(out, "static constexpr auto kEnd = (wrapper_t)(~(underlying_t)0);");
        cxx_nl(out);
        cxx_writeln(out, "class Iterator {");
        cxx_enter(out);
        cxx_writeln(out, "wrapper_t m_value;");
        cxx_privacy(out, "public");
        cxx_writeln(out, "constexpr Iterator(wrapper_t value) : m_value(value) { }");
        cxx_writeln(out, "constexpr Iterator& operator++() { m_value = (wrapper_t)((underlying_t)m_value + 1); return *this; }");
        cxx_writeln(out, "constexpr const Iterator operator++(int) { Iterator it = *this; ++(*this); return it; }");
        cxx_writeln(out, "constexpr bool operator==(const Iterator& other) const { return m_value == other.m_value; }");
        cxx_writeln(out, "constexpr bool operator!=(const Iterator& other) const { return m_value != other.m_value; }");
        cxx_writeln(out, "constexpr %s operator*() const { return m_value; }", get_name());
        cxx_leave(out);
        cxx_writeln(out, "};");
        cxx_nl(out);
        cxx_writeln(out, "class Range {");
        cxx_enter(out);
        cxx_writeln(out, "wrapper_t m_begin;");
        cxx_writeln(out, "wrapper_t m_end;");
        cxx_privacy(out, "public");
        cxx_writeln(out, "constexpr Range(wrapper_t begin, wrapper_t end) : m_begin(begin), m_end(end) { }");
        cxx_writeln(out, "constexpr Iterator begin() const { return Iterator(m_begin); }");
        cxx_writeln(out, "constexpr Iterator end() const { return Iterator(m_end); }");
        cxx_leave(out);
        cxx_writeln(out, "};");
        cxx_nl(out);

        cxx_writeln(out, "static constexpr Range range(wrapper_t begin, wrapper_t end) { return Range(begin, end); }");
    }

    cxx_writeln(out, "constexpr operator wrapper_t() const { return m_value; }");

    cxx_writeln(out, "constexpr underlying_t as_integral() const { return (underlying_t)m_value; }");
    cxx_writeln(out, "constexpr wrapper_t as_enum() const { return m_value; }");
    if (is_facade)
    {
        cxx_writeln(out, "constexpr facade_t as_facade() const { return (facade_t)m_value; }");
    }

    if (is_lookup)
    {
        cxx_nl(out);
        // generate min and max values
        cxx_writeln(out, "static constexpr auto kMin = ((underlying_t)%s);", mpz_get_str(nullptr, 10, lowest));
        cxx_writeln(out, "static constexpr auto kMax = ((underlying_t)%s);", mpz_get_str(nullptr, 10, highest));

        mpz_add_ui(highest, highest, 1);
        cxx_writeln(out, "static constexpr auto kCount = ((underlying_t)%s);", mpz_get_str(nullptr, 10, highest));

        // generate implicit checked conversion to underlying integral type
        cxx_writeln(out, "constexpr operator underlying_t() const { return as_integral(); }");
    }

    cxx_nl(out);
    cxx_writeln(out, "constexpr bool operator==(wrapper_t other) const { return m_value == other; }");
    cxx_writeln(out, "constexpr bool operator!=(wrapper_t other) const { return m_value != other; }");

    if (!is_bitflags && !is_arithmatic && !is_lookup)
    {
        emit_default_is_valid(out);
    }
    else if (is_lookup)
    {
        // check that we are within the range of the enum
        cxx_writeln(out, "constexpr bool is_valid() const { return as_integral() >= kMin && as_integral() <= kMax; }");
    }

    if (is_bitflags)
    {
        String flags;
        m_cases.foreach([&](auto c)
        {
            if (!flags.empty())
                flags += " | ";
            flags += refl_fmt("e%s", c->get_name());
        });
        cxx_writeln(out, "static constexpr %s none() { return %s((wrapper_t)0); };", get_name(), get_name());
        cxx_writeln(out, "static constexpr %s mask() { return %s(%.*s); };", get_name(), get_name(), (int)flags.size(), flags.c_str());
        // emit bitwise operators
        cxx_nl(out);
        cxx_writeln(out, "constexpr %s operator~() const { return ~m_value; }", get_name());
        cxx_writeln(out, "constexpr %s operator|(const %s& other) const { return m_value | other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s operator&(const %s& other) const { return m_value & other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s operator^(const %s& other) const { return m_value ^ other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator|=(const %s& other) { m_value = m_value | other.m_value; return *this; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator&=(const %s& other) { m_value = m_value & other.m_value; return *this; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator^=(const %s& other) { m_value = m_value ^ other.m_value; return *this; }", get_name(), get_name());

        cxx_writeln(out, "constexpr bool test(wrapper_t other) const { return (m_value & other) != none(); }");
        cxx_writeln(out, "constexpr bool any(wrapper_t other) const { return (m_value & other) != none(); }");
        cxx_writeln(out, "constexpr bool all(wrapper_t other) const { return (m_value & other) == other; }");
        cxx_writeln(out, "constexpr bool none(wrapper_t other) const { return (m_value & other) == none(); }");
        cxx_writeln(out, "constexpr %s& set(wrapper_t other) { m_value = m_value | other; return *this; }", get_name());
        cxx_writeln(out, "constexpr %s& reset(wrapper_t other) { m_value = m_value & ~other; return *this; }", get_name());
        cxx_writeln(out, "constexpr %s& flip(wrapper_t other) { m_value = m_value ^ other; return *this; }", get_name());

        // is_valid is defined as no invalid flags set
        cxx_writeln(out, "constexpr bool is_valid() const { return (m_value & ~mask()) == none(); }");
    }

    if (is_arithmatic)
    {
        // implement arithmatic operators
        cxx_nl(out);
        cxx_writeln(out, "constexpr %s operator+(const %s& other) const { return m_value + other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s operator-(const %s& other) const { return m_value - other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s operator*(const %s& other) const { return m_value * other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s operator/(const %s& other) const { return m_value / other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s operator%(const %s& other) const { return m_value %% other.m_value; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator+=(const %s& other) { m_value = m_value + other.m_value; return *this; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator-=(const %s& other) { m_value = m_value - other.m_value; return *this; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator*=(const %s& other) { m_value = m_value * other.m_value; return *this; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator/=(const %s& other) { m_value = m_value / other.m_value; return *this; }", get_name(), get_name());
        cxx_writeln(out, "constexpr %s& operator%=(const %s& other) { m_value = m_value %% other.m_value; return *this; }", get_name(), get_name());

        // is_valid is not defined for arithmatic types
    }

    if (is_ordered)
    {
        cxx_nl(out);
        cxx_writeln(out, "constexpr bool operator<(const %s& other) const { return m_value < other.m_value; }", get_name());
        cxx_writeln(out, "constexpr bool operator<=(const %s& other) const { return m_value <= other.m_value; }", get_name());
        cxx_writeln(out, "constexpr bool operator>(const %s& other) const { return m_value > other.m_value; }", get_name());
        cxx_writeln(out, "constexpr bool operator>=(const %s& other) const { return m_value >= other.m_value; }", get_name());
    }

    emit_methods(out, ePrivacyPublic);

    emit_end_record(out);
    cxx_nl(out);
    cxx_writeln(out, "static_assert(sizeof(%s) == sizeof(%s::underlying_t), \"%s size mismatch\");", get_name(), get_name(), get_name());
}

static const char *layout_enum_name(ref_attrib_tag_t tag)
{
    switch (tag)
    {
    case eAttribLayoutStable: return "eLayoutStable";
    case eAttribLayoutPacked: return "eLayoutPacked";
    case eAttribLayoutSystem: return "eLayoutSystem";
    case eAttribLayoutAny: return "eLayoutAny";
    case eAttribLayoutConstBuffer: return "eLayoutConstBuffer";

    default: NEVER("invalid layout %d", tag);
    }
}

void Decl::emit_name_info(Sema& sema, cxx_emit_t *out, const char *id, const ref_ast_t *ast) const
{
    mpz_t typeid_value;
    get_type_id(sema, ast, typeid_value);

    cxx_writeln(out, "static constexpr ObjectName kFullName = impl::objname(\"%s\");", id);
    cxx_writeln(out, "static constexpr ObjectName kName = impl::objname(\"%s\");", get_name());
    cxx_writeln(out, "static constexpr ObjectId kTypeId = %s;", mpz_get_str(nullptr, 10, typeid_value));
    cxx_nl(out);

    auto layout = get_layout();
    const char *layout_name = layout_enum_name(layout);

    cxx_writeln(out, "static constexpr TypeLayout kTypeLayout = %s;", layout_name);
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

static const char* attribs_name(ref_ast_t *ast)
{
    if (has_attrib_tag(ast->attributes, eAttribTransient))
        return "eAttribTransient";

    return "eAttribNone";
}

static void emit_record_fields(cxx_emit_t *out, const Vector<Field*>& fields)
{
    cxx_writeln(out, "static constexpr field_t kFields[%zu] = {", fields.size());
        cxx_enter(out);
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields.get(i);
            cxx_writeln(out, "field_t {");
            cxx_enter(out);
            cxx_writeln(out, ".name    = impl::objname(\"%s\"),", f->get_name());
            cxx_writeln(out, ".index   = %zu,", i);
            cxx_writeln(out, ".access  = %s,", access_name(f->get_privacy()));
            cxx_writeln(out, ".attribs = %s", attribs_name(f->get_ast()));
            cxx_leave(out);
            cxx_writeln(out, "},");
        }
        cxx_leave(out);
    cxx_writeln(out, "};");
}

static void emit_record_visit(cxx_emit_t *out, const char* id, const Vector<Field*>& fields)
{
    cxx_writeln(out, "constexpr auto visit_field(%s& object, const field_t& field, auto&& fn) const {", id);
    cxx_enter(out);
        cxx_writeln(out, "switch (field.index) {");
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields.get(i);
            cxx_writeln(out, "case %zu: return fn(object.%s);", i, f->get_name());
        }
        cxx_writeln(out, "default: return fn(ctu::OutOfBounds{field.index});");
        cxx_writeln(out, "}");
    cxx_leave(out);
    cxx_writeln(out, "};");
    cxx_nl(out);
    cxx_writeln(out, "constexpr void foreach(%s& object, auto&& fn) const {", id);
    cxx_enter(out);
        for (size_t i = 0; i < fields.size(); ++i)
        {
            auto f = fields.get(i);
            cxx_writeln(out, "fn(kFields[%zu], object.%s);", i, f->get_name());
        }
    cxx_leave(out);
    cxx_writeln(out, "};");
}

static void emit_ctor(cxx_emit_t *out)
{
    cxx_writeln(out, "consteval TypeInfo() : TypeInfoBase(kName, sizeof(type_t), alignof(type_t), kTypeId) { }");
}

static void emit_reflect_hook(cxx_emit_t *out, const char* id)
{
    cxx_writeln(out, "template<> consteval auto reflect<%s>() {", id);
    cxx_enter(out);
    cxx_writeln(out, "return TypeInfo<%s>{};", id);
    cxx_leave(out);
    cxx_writeln(out, "}");
}

static void emit_info_header(cxx_emit_t *out, const char* id)
{
    cxx_writeln(out, "template<> class TypeInfo<%s> : public TypeInfoBase {", id);
    cxx_writeln(out, "public:");
}

static const char* get_decl_name(ref_ast_t *ast, Sema& sema, const char *name)
{
    bool is_external = type_is_external(ast);
    if (is_external)
    {
        return name;
    }
    else
    {
        return refl_fmt("%s::%s", sema.get_namespace(), name);
    }
}

void RecordType::emit_serialize(cxx_emit_t *out, const char *id, const Vector<Field*>& fields) const
{
    CT_UNUSED(out);
    CT_UNUSED(id);
    CT_UNUSED(fields);
}

void Struct::emit_reflection(Sema& sema, cxx_emit_t *out) const
{
    if (type_is_internal(m_ast))
        return;

    auto id = get_decl_name(m_ast, sema, get_name());

    mpz_t typeid_value;
    get_type_id(sema, m_ast, typeid_value);

    emit_info_header(out, id);
        cxx_enter(out);
        cxx_writeln(out, "using type_t = %s;", id);
        cxx_writeln(out, "using field_t = ctu::ObjectField;");
        cxx_writeln(out, "using Type = type_t;");
        cxx_writeln(out, "using Field = field_t;");
        cxx_nl(out);
        emit_name_info(sema, out, id, m_ast);
        cxx_nl(out);
        emit_record_fields(out, m_fields);
        cxx_nl(out);
        emit_ctor(out);
        cxx_nl(out);
        emit_record_visit(out, id, m_fields);
        emit_serialize(out, id, m_fields);
        cxx_leave(out);
    cxx_writeln(out, "};");
    cxx_nl(out);
    emit_reflect_hook(out, id);
    cxx_nl(out);
}

void Class::emit_reflection(Sema& sema, cxx_emit_t *out) const
{
    if (type_is_internal(m_ast))
        return;

    auto id = get_decl_name(m_ast, sema, get_name());
    auto parent = m_parent ? m_parent->get_cxx_name(nullptr) : "void";

    mpz_t typeid_value;
    get_type_id(sema, m_ast, typeid_value);

    emit_info_header(out, id);
        cxx_enter(out);
        cxx_writeln(out, "using type_t = %s;", id);
        cxx_writeln(out, "using super_t = %s;", parent);
        cxx_writeln(out, "using field_t = ctu::ObjectField;");
        cxx_writeln(out, "using method_t = ctu::ObjectMethod;");
        cxx_writeln(out, "using Type = type_t;");
        cxx_writeln(out, "using Super = super_t;");
        cxx_writeln(out, "using Field = field_t;");
        cxx_writeln(out, "using Method = method_t;");
        cxx_nl(out);
        emit_name_info(sema, out, id, m_ast);
        cxx_writeln(out, "static constexpr bool kHasSuper = %s;", m_parent != nullptr ? "true" : "false");
        cxx_writeln(out, "static constexpr TypeInfo<%s> kSuper{};", parent);
        cxx_nl(out);
        emit_record_fields(out, m_fields);
        cxx_nl(out);
        cxx_writeln(out, "// methods");
        cxx_writeln(out, "static constexpr method_t kMethods[%zu] = {", m_methods.size());
            cxx_enter(out);
            for (size_t i = 0; i < m_methods.size(); ++i)
            {
                auto m = m_methods.get(i);
                cxx_writeln(out, "method_t { .name = impl::objname(\"%s\"), .index = %zu },", m->get_repr(), i);
            }
            cxx_leave(out);
        cxx_writeln(out, "};");
        emit_ctor(out);
        cxx_nl(out);
        emit_record_visit(out, id, m_fields);
        emit_serialize(out, id, m_fields);
        cxx_leave(out);
    cxx_writeln(out, "};");
    cxx_nl(out);
    emit_reflect_hook(out, id);
    cxx_nl(out);
}

size_t Variant::max_tostring() const {
    if (has_attrib_tag(m_ast->attributes, eAttribBitflags))
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
    // sum all the cases + 2 for each comma and space
    size_t max = 0;

    m_cases.foreach([&](auto c)
    {
        size_t len = strlen(c->get_name());
        max += len + 2;
    });

    return max;
}

Case *Variant::get_zero_case() const
{
    if (m_cases.size() == 0)
        return nullptr;

    for (size_t i = 0; i < m_cases.size(); ++i)
    {
        auto c = m_cases.get(i);
        mpz_t id;
        if (c->get_integer(id))
        {
            if (mpz_cmp_ui(id, 0) == 0)
                return c;
        }
    }

    return nullptr;
}

void Variant::emit_reflection(Sema& sema, cxx_emit_t *out) const
{
    if (type_is_internal(m_ast))
        return;

    auto id = get_decl_name(m_ast, sema, get_name());

    mpz_t typeid_value;
    get_type_id(sema, m_ast, typeid_value);
    bool is_bitflags = has_attrib_tag(m_ast->attributes, eAttribBitflags);

    size_t max_tostring_length = max_tostring();

    emit_info_header(out, id);
        cxx_enter(out);
        cxx_writeln(out, "using type_t = %s;", id);
        cxx_writeln(out, "using underlying_t = %s::underlying_t;", id);
        cxx_writeln(out, "using case_t = ctu::EnumCase<%s>;", id);
        cxx_nl(out);
        cxx_writeln(out, "static constexpr size_t kMaxLength = %zu;", max_tostring_length);
        cxx_writeln(out, "using string_t = SmallString<kMaxLength>;");
        cxx_nl(out);
        cxx_writeln(out, "using Type = type_t;");
        cxx_writeln(out, "using Underlying = underlying_t;");
        cxx_writeln(out, "using Case = case_t;");
        cxx_writeln(out, "using String = string_t;");
        cxx_nl(out);
        emit_name_info(sema, out, id, m_ast);
        if (m_parent)
            cxx_writeln(out, "static constexpr TypeInfo<underlying_t> kUnderlying{};");
        else
            cxx_writeln(out, "static constexpr TypeInfo<void> kUnderlying{};");

        cxx_writeln(out, "static constexpr bool kHasDefault = %s;", m_default_case != nullptr ? "true" : "false");
        if (m_default_case)
            cxx_writeln(out, "static constexpr %s kDefaultCase = %s::e%s;", id, id, m_default_case->get_name());

        cxx_nl(out);
        cxx_writeln(out, "static constexpr case_t kCases[%zu] = {", m_cases.size());
            cxx_enter(out);
            m_cases.foreach([&](auto c) {
                cxx_writeln(out, "case_t { impl::objname(\"%s\"), %s::e%s },", c->get_repr(), id, c->get_name());
            });
            cxx_leave(out);
        cxx_writeln(out, "};");
        cxx_nl(out);
        emit_ctor(out);
        cxx_nl(out);

        cxx_nl(out);
        cxx_writeln(out, "constexpr string_t to_string(type_t value, [[maybe_unused]] int base = 10) const {");
        cxx_enter(out);
        if (is_bitflags)
        {
            Case *zero = get_zero_case();
            cxx_writeln(out, "string_t result;");
            if (zero != nullptr)
            {
                cxx_writeln(out, "if (value == %s::e%s) return impl::objname(\"%s\");", id, zero->get_name(), zero->get_repr());
            }
            cxx_writeln(out, "bool first = true;");
            cxx_writeln(out, "for (auto option : kCases) {");
            cxx_enter(out);
            if (zero != nullptr)
            {
                cxx_writeln(out, "if ((option.value != %s::e%s) && (value & option.value) == option.value) {", id, zero->get_name());
            }
            else
            {
                cxx_writeln(out, "if ((value & option.value) == option.value) {");
            }
            cxx_enter(out);
            cxx_writeln(out, "if (!first) result += \", \";");
            cxx_writeln(out, "result += option.name;");
            cxx_writeln(out, "first = false;");
            cxx_leave(out);
            cxx_writeln(out, "}");
            cxx_leave(out);
            cxx_writeln(out, "}");
            cxx_writeln(out, "return result;");
        }
        else
        {
            cxx_writeln(out, "for (auto option : kCases) {");
            cxx_enter(out);
            cxx_writeln(out, "if (option.value == value) return option.name;");
            cxx_leave(out);
            cxx_writeln(out, "}");
            cxx_writeln(out, "return string_t(value.as_integral(), base);");
        }
        cxx_leave(out);
        cxx_writeln(out, "};");
    cxx_leave(out);
    cxx_writeln(out, "};");

    cxx_nl(out);
    emit_reflect_hook(out, id);
    cxx_nl(out);
}
