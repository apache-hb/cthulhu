#pragma once

#include "arena/arena.h"
#include "base/panic.h"
#include "base/util.h"
#include "io/io.h"
#include "memory/memory.h"
#include "notify/notify.h"
#include "ref/ast.h"
#include "ref/eval.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

typedef struct logger_t logger_t;
typedef struct vector_t vector_t;
typedef struct ref_ast_t ref_ast_t;
typedef struct io_t io_t;
typedef struct arena_t arena_t;

typedef struct ref_emit_t
{
    const char *file;

    vector_t *prelude;

    logger_t *logger;
    ref_ast_t *ast;
    io_t *header;
    io_t *source;
} ref_emit_t;

namespace refl { class Sema; }

char *refl_fmt(const char *fmt, auto&&... args) {
    return ::str_format(get_global_arena(), fmt, args...);
}

template<typename T>
class Vector {
    vector_t *m_vec;

public:
    Vector(size_t size, arena_t *arena)
        : m_vec(vector_new(size, arena))
    { }

    Vector(vector_t *vec)
        : m_vec(vec)
    { }

    T get(size_t index) const {
        return (T)vector_get(m_vec, index);
    }

    void set(size_t index, T value) {
        vector_set(m_vec, index, (void*)value);
    }

    void push(T value) {
        ::vector_push(&m_vec, (void*)value);
    }

    void pop() { vector_drop(m_vec); }

    size_t find(T value) {
        return vector_find(m_vec, (const void*)value);
    }

    size_t size() const {
        return vector_len(m_vec);
    }

    void foreach(auto&& fn) const {
        for (size_t i = 0; i < size(); i++) {
            fn((T)vector_get(m_vec, i));
        }
    }
};

using vector_str_t = Vector<const char*>;

class String {
    typevec_t *m_data;

public:
    String()
        : m_data(typevec_new(sizeof(char), 256, get_global_arena()))
    { }

    const char *c_str() const {
        return (const char*)typevec_data(m_data);
    }

    size_t size() const {
        return typevec_len(m_data);
    }

    void append(const char *str) {
        typevec_append(m_data, str, ctu_strlen(str));
    }

    String& operator+=(const char *str) {
        append(str);
        return *this;
    }

    bool empty() const {
        return size() == 0;
    }
};

template<typename K, typename V>
class Map {
    map_t *m_map;

public:
    Map(size_t size, arena_t *arena)
        : m_map(map_optimal(size, kTypeInfoString, arena))
    { }

    Map(map_t *map)
        : m_map(map)
    { }

    V get(const K& key) const {
        return (V)map_get(m_map, (const void*)key);
    }

    void set(const K& key, V value) {
        map_set(m_map, (const void*)key, (void*)value);
    }

    void foreach(auto&& fn) {
        map_iter_t iter = map_iter(m_map);
        while (map_has_next(&iter)) {
            map_entry_t entry = map_next(&iter);
            fn((K)entry.key, (V)entry.value);
        }
    }
};

struct out_t
{
    refl::Sema *sema;

    typevec_t *m_data = typevec_new(sizeof(char), 0x1000, get_global_arena());
    size_t m_depth = 0;

    out_t(refl::Sema *i)
        : sema(i)
    { }

    void dump(io_t *io)
    {
        io_write(io, (void*)typevec_data(m_data), typevec_len(m_data));
    }

    void enter() { m_depth += 1; }
    void leave() { CTASSERT(m_depth > 0); m_depth -= 1; }

    void indent()
    {
        for (size_t i = 0; i < m_depth; i++) { typevec_append(m_data, "    ", 4); }
    }

    void writeln(const char *fmt, auto&&... args)
    {
        indent();
        write(fmt, args...);
        nl();
    }

    void write(const char *fmt, auto&&... args)
    {
        auto it = text_format(get_global_arena(), fmt, args...);
        typevec_append(m_data, it.text, it.length);
    }

    void nl()
    {
        typevec_push(m_data, "\n");
    }
};

namespace refl {
    enum TreeKind {
        eKindProgram,

        eKindClass,
        eKindVariant,
        eKindAlias,

        eKindField,
        eKindParam,
        eKindCase,
        eKindMethod,

        eKindTypeTemplate,
        eKindTypeInstance,

        eKindTypeBool,
        eKindTypeMemory,
        eKindTypeString,
        eKindTypeVoid,
        eKindTypeByte,
        eKindTypeInt,
        eKindTypeFloat,
        eKindTypePointer,
        eKindReference,
        eKindTypeOpaque,
        eKindTypeAlias
    };

    class Tree;
    class Decl;
    class Type;
    class Module;
    class Sema;
    class Expr;

    class ResolveStack {
        Vector<Decl*> m_stack { 32, get_global_arena() };

    public:
        bool enter_decl(Sema& sema, Decl *decl);
        void leave_decl();
    };

    class DeclDepends {
    public:
        Vector<Decl*> m_depends;
        void add(Decl *decl) {
            CTASSERTF(decl != nullptr, "decl is null");
            if (m_depends.find(decl) == SIZE_MAX)
                m_depends.push(decl);
        }
    };

    using declmap_t = Map<const char*, Decl*>;

    declmap_t get_builtin_types();

    class Sema {
        Sema *m_parent;
        ResolveStack *m_resolve;
        logger_t *m_logger;
        declmap_t m_decls;

        const char* m_namespace;
        const char *m_api = nullptr;

        vector_str_t imports { 32, get_global_arena() };

    public:
        Sema(const Sema&) = delete;
        Sema& operator=(const Sema&) = delete;

        const char *get_api() const { return m_api; }
        logger_t *get_logger() const { return m_logger; }

        const char* get_namespace() const { return m_namespace; }

        Sema(Sema *parent)
            : m_parent(parent)
            , m_resolve(parent->m_resolve)
            , m_logger(parent->m_logger)
            , m_decls(parent->m_decls)
        { }

        Sema(logger_t *logger)
            : m_parent(nullptr)
            , m_resolve(new ResolveStack())
            , m_logger(logger)
            , m_decls(get_builtin_types())
        { }

        void add_decl(const char *name, Decl *decl);
        Decl *get_decl(const char *name) const;

        void import_module(ref_ast_t *mod);

        void forward_module(ref_ast_t *mod);
        void resolve_all();

        Type *resolve_type(ref_ast_t *ast);

        void emit_all(io_t *header, const char *file);

        Decl *forward_decl(const char *name, ref_ast_t *ast);

        event_builder_t report(const diagnostic_t *diag, const node_t *node, const char *fmt, auto&&... args)
        {
            return msg_notify(m_logger, diag, node, fmt, args...);
        }

        template<typename T>
        T *resolve_decl(T *decl)
        {
            CTASSERT(decl != nullptr);

            if (decl->is_resolved())
                return decl;

            if (!m_resolve->enter_decl(*this, decl))
                return nullptr;

            decl->resolve(*this);
            m_resolve->leave_decl();

            return decl;
        }
    };

    class Tree {
        const node_t *m_node;
        TreeKind m_kind;

    protected:
        Tree(const node_t *node, TreeKind kind)
            : m_node(node)
            , m_kind(kind)
        { }

    public:
        virtual ~Tree() = default;

        const node_t *get_node() const { return m_node; }
        TreeKind get_kind() const { return m_kind; }

        virtual bool is_type() const { return false; }
        virtual bool is_decl() const { return false; }
        virtual bool is_expr() const { return false; }
    };

    class Decl : public Tree {
        const char *m_name;
        bool m_resolved = false;
        Type *m_type = nullptr;

    protected:
        void set_type(Type *type) { m_type = type; }
        void finish_resolve() { m_resolved = true; }

        Decl(const node_t *node, TreeKind kind, const char *name)
            : Tree(node, kind)
            , m_name(name)
        { }

    public:
        const char *get_name() const { return m_name; }
        virtual const char *get_repr() const { return m_name; }
        bool is_resolved() const { return m_resolved; }
        Type *get_type() const { return m_type; }

        virtual void resolve(Sema& sema) = 0;
        virtual void resolve_type(Sema& sema) { resolve(sema); }

        virtual void emit_proto(out_t&) const { }
        virtual void emit_impl(out_t&) const { }
        virtual void emit_reflection(Sema&, out_t&) const { }

        virtual void get_deps(DeclDepends&) const { }

        bool is_decl() const override { return true; }
        bool is_template() const { return get_kind() == eKindTypeTemplate; }
    };

    class Type : public Decl {
        size_t m_sizeof = 0;
        size_t m_alignof = 0;

    protected:
        void set_sizeof(size_t size) { m_sizeof = size; }
        void set_alignof(size_t align) { m_alignof = align; }

    public:
        Type(const node_t *node, TreeKind kind, const char *name, size_t size, size_t align)
            : Decl(node, kind, name)
            , m_sizeof(size)
            , m_alignof(align)
        { }

        virtual const char* get_cxx_name(const char *) const { return ""; }

        size_t get_sizeof() const { return m_sizeof; }
        size_t get_alignof() const { return m_alignof; }

        bool is_type() const override { return true; }
        bool is_record_type() const { return get_kind() == eKindClass || get_kind() == eKindVariant; }
        virtual const char *get_opaque_name() const { return nullptr; }
    };

    class OpaqueType : public Type {
    public:
        OpaqueType(const node_t *node, const char *name)
            : Type(node, eKindTypeOpaque, name, sizeof(void*), alignof(void*))
        { CTASSERT(name != nullptr); }

        void resolve(Sema&) override { finish_resolve(); }

        const char* get_cxx_name(const char *name) const override {
            return (name == nullptr) ? get_name() : refl_fmt("%s %s", get_name(), name);
        }

        const char *get_opaque_name() const override { return get_name(); }
    };

    class AddressType : public Type {
        Type *m_pointee = nullptr;
        const char *m_symbol = nullptr;

    public:
        AddressType(const node_t *node, TreeKind kind, Type *pointee, const char *symbol)
            : Type(node, kind, nullptr, sizeof(void*), alignof(void*))
            , m_pointee(pointee)
            , m_symbol(symbol)
        { }

        void resolve(Sema&) override { }

        const char* get_cxx_name(const char *name) const override {
            auto inner =  m_pointee->get_cxx_name(nullptr);
            return (name == nullptr) ? refl_fmt("%s%s", inner, m_symbol)
                                     : refl_fmt("%s %s%s", inner, m_symbol, name);
        }
    };

    class PointerType : public AddressType {
    public:
        PointerType(const node_t *node, Type *pointee)
            : AddressType(node, eKindTypePointer, pointee, "*")
        { }
    };

    class ReferenceType : public AddressType {
    public:
        ReferenceType(const node_t *node, Type *pointee)
            : AddressType(node, eKindReference, pointee, "&")
        { }
    };

    class IntType : public Type {
        digit_t m_digit;
        sign_t m_sign;

    public:
        IntType(const char *name, digit_t digit, sign_t sign);

        void resolve(Sema&) override { finish_resolve(); }

        const char* get_cxx_name(const char *name) const override;

        digit_t get_digit() const { return m_digit; }
        sign_t get_sign() const { return m_sign; }
    };

    class FloatType : public Type {
    public:
        FloatType(const char *name)
            : Type(node_builtin(), eKindTypeFloat, name, sizeof(float), alignof(float))
        { }

        const char* get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "float" : refl_fmt("float %s", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class BoolType : public Type {
    public:
        BoolType(const char *name)
            : Type(node_builtin(), eKindTypeBool, name, sizeof(bool), alignof(bool))
        { }

        const char* get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "bool" : refl_fmt("bool %s", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class StringType : public Type {
    public:
        StringType(const char *name)
            : Type(node_builtin(), eKindTypeString, name, sizeof(const char*), alignof(const char*))
        { }

        void resolve(Sema&) override { finish_resolve(); }

        const char* get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "const char*" : refl_fmt("const char *%s", name);
        }
    };

    class VoidType : public Type {
    public:
        VoidType(const char *name)
            : Type(node_builtin(), eKindTypeVoid, name, 0, 0)
        { }

        const char* get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "void" : refl_fmt("void %s", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class MemoryType : public Type {
    public:
        MemoryType(const char *name)
            : Type(node_builtin(), eKindTypeMemory, name, sizeof(void*), alignof(void*))
        { }

        const char* get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "void*" : refl_fmt("void *%s", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class ConstType : public Type {
        Type *m_inner = nullptr;
    public:
        ConstType(const node_t *node, Type *inner)
            : Type(node, eKindTypeAlias, nullptr, inner->get_sizeof(), inner->get_alignof())
            , m_inner(inner)
        { }

        void resolve(Sema&) override { finish_resolve(); }

        const char* get_cxx_name(const char *name) const override
        {
            if (m_inner->get_kind() == eKindTypeMemory) {
                return (name == nullptr) ? "const void*" : refl_fmt("const void *%s", name);
            }
            return (name == nullptr) ? m_inner->get_cxx_name("const") : refl_fmt("const %s %s", m_inner->get_cxx_name(nullptr), name);
        }
    };

    class TreeBackedDecl : public Decl {
    protected:
        ref_ast_t *m_ast = nullptr;

        TreeBackedDecl(ref_ast_t *ast, TreeKind kind)
            : Decl(ast->node, kind, ast->name)
            , m_ast(ast)
        { }

    public:
        const char *get_repr() const override;
    };

    class Field : public TreeBackedDecl {
    public:
        Field(ref_ast_t *ast);

        void resolve(Sema& sema) override;
        void resolve_type(Sema& sema) override;

        void emit_impl(out_t& out) const override;
        void emit_field(out_t& out) const;

        ref_privacy_t get_privacy() const { return m_ast->privacy; }
        bool is_transient() const;

        ref_ast_t *get_ast() const { return m_ast; }
    };

    class Param : public TreeBackedDecl {
    public:
        Param(ref_ast_t *ast)
            : TreeBackedDecl(ast, eKindParam)
        { }

        void resolve(Sema& sema) override {
            Type *type = sema.resolve_type(m_ast->type);
            if (type == nullptr) return;
            set_type(type);
            finish_resolve();
        }
    };

    class RecordType;

    class Method : public TreeBackedDecl {
        Vector<Param*> m_params { 32, get_global_arena() };

        Type *m_return = nullptr;

        bool m_thunk = false;

    public:
        Method(ref_ast_t *ast)
            : TreeBackedDecl(ast, eKindMethod)
        { }

        void resolve(Sema& sema) override;

        void emit_impl(out_t& out) const override;
        void emit_thunk(out_t& out) const;
        void emit_method(out_t& out, const RecordType& parent) const;

        void resolve_deps(DeclDepends& deps) const {
            if (m_return) deps.add(m_return);
            m_params.foreach([&](Param *param) { deps.add(param->get_type()); });
        }

        bool should_emit_thunk() const { return m_thunk; }

        ref_privacy_t get_privacy() const { return m_ast->privacy; }
    };

    class Case : public TreeBackedDecl {
        eval_result_t m_eval = eEvalInvalid;
        mpz_t digit_value = {};

    public:
        Case(ref_ast_t *ast);

        const char *get_repr() const override;

        void resolve(Sema& sema) override;

        void emit_impl(out_t& out) const override;

        void emit_value(out_t& out) const;
        void emit_case(out_t& out) const;

        const char* get_value() const;

        bool get_integer(mpz_t out) const;
    };

    class TypeAlias : public Type {
        ref_ast_t *m_ast = nullptr;
    public:
        TypeAlias(ref_ast_t *ast)
            : Type(ast->node, eKindTypeAlias, ast->name, 0, 0)
            , m_ast(ast)
        { }

        void resolve(Sema& sema) override {
            if (is_resolved()) return;
            finish_resolve();

            Type *type = sema.resolve_type(m_ast->type);

            set_type(type);
            set_alignof(type->get_alignof());
            set_sizeof(type->get_sizeof());
        }

        const char *get_cxx_name(const char *name) const override {
            Type *type = get_type();
            return type->get_cxx_name(name);
        }
    };

    class RecordType : public Type {
        const char *m_record = nullptr;

    protected:
        ref_ast_t *m_ast = nullptr;
        Vector<Method*> m_methods { 32, get_global_arena() };
        Type *m_parent = nullptr;

        RecordType(ref_ast_t *ast, TreeKind kind, const char *record)
            : Type(ast->node, kind, ast->name, 0, 0)
            , m_record(record)
            , m_ast(ast)
        { }

        ref_privacy_t emit_methods(out_t& out, ref_privacy_t privacy) const;

        void emit_begin_record(out_t& out, bool write_parent = true) const;
        void emit_ctors(out_t& out) const;
        ref_privacy_t emit_dtors(out_t& out, ref_privacy_t privacy) const;

        void emit_end_record(out_t& out) const;
        ref_privacy_t emit_fields(out_t& out, const Vector<Field*>& fields, ref_privacy_t privacy) const;

        void emit_serialize(out_t& out, const char *id, const Vector<Field*>& fields) const;

    public:
        void resolve(Sema& sema) override;

        void emit_proto(out_t& out) const override;

        const char* get_cxx_name(const char *name) const override {
            return (name == nullptr) ? get_name() : refl_fmt("%s %s", get_name(), name);
        }

        void get_deps(DeclDepends& deps) const override {
            if (m_parent != nullptr)
                deps.add(m_parent);

            m_methods.foreach([&](Method *method) {
                method->resolve_deps(deps);
            });
        }

        bool is_virtual() const { return m_ast->flags & eDeclVirtual; }
        bool is_final() const { return m_ast->flags & eDeclSealed; }
        bool is_stable_layout() const;
    };

    class Class final : public RecordType {
        Vector<Field*> m_fields { 32, get_global_arena() };
    public:
        Class(ref_ast_t *ast);

        void resolve(Sema& sema) override;

        void emit_impl(out_t& out) const override;

        void emit_reflection(Sema& sema, out_t& out) const override;

        void get_deps(DeclDepends& deps) const override {
            RecordType::get_deps(deps);
            m_fields.foreach([&](Field *field) {
                deps.add(field->get_type());
            });
        }
    };

    class Struct final : public RecordType {
        Vector<Field*> m_fields { 32, get_global_arena() };
    public:
        Struct(ref_ast_t *ast);

        void resolve(Sema& sema) override;

        void emit_impl(out_t& out) const override;

        void emit_reflection(Sema& sema, out_t& out) const override;

        void get_deps(DeclDepends& deps) const override {
            RecordType::get_deps(deps);
            m_fields.foreach([&](Field *field) {
                deps.add(field->get_type());
            });
        }
    };

    class Variant : public RecordType {
        Vector<Case*> m_cases { 32, get_global_arena() };

        Case *m_default_case = nullptr;
    public:
        Variant(ref_ast_t *ast);

        void resolve(Sema& sema) override;

        void emit_impl(out_t& out) const override;
        void emit_facade(out_t& out) const;

        void emit_reflection(Sema& sema, out_t& out) const override;
        Case *get_zero_case() const;

        void emit_default_is_valid(out_t& out) const;

        size_t max_tostring() const;

        size_t max_tostring_bitflags() const;
    };

    class Expr : public Tree {
    public:
        Type *m_type;
    };

    void emit_module(Module *module, io_t *header, io_t *source);
}
