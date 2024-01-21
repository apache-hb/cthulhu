#pragma once

#include "base/panic.h"
#include "cthulhu/events/events.h"
#include "io/io.h"
#include "notify/notify.h"
#include "ref/ast.h"
#include "scan/node.h"

#include <map>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <format>
#include <sstream>

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

struct out_t
{
    std::stringstream m_stream;
    size_t m_depth = 0;

    void dump(io_t *io)
    {
        auto str = m_stream.str();
        io_write(io, str.data(), str.size());
    }

    void enter() { m_depth += 1; }
    void leave() { CTASSERT(m_depth > 0); m_depth -= 1; }

    void indent()
    {
        for (size_t i = 0; i < m_depth; i++) { m_stream << "    "; }
    }

    void writeln(const char *fmt, auto&&... args)
    {
        indent();
        m_stream << std::vformat(fmt, std::make_format_args(args...));
        nl();
    }

    void write(const char *fmt, auto&&... args)
    {
        m_stream << std::vformat(fmt, std::make_format_args(args...));
    }

    void nl()
    {
        m_stream << "\n";
    }
};

namespace refl {
    enum TreeKind {
        eKindProgram,

        eKindClass,
        eKindStruct,
        eKindVariant,
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
        eKindTypeOpaque
    };

    class Tree;
    class Decl;
    class Type;
    class Module;
    class Sema;
    class Expr;

    class ResolveStack {
        std::vector<Decl*> m_stack;

    public:
        bool enter_decl(Sema& sema, Decl *decl);
        void leave_decl();
    };

    class DeclDepends {
    public:
        std::vector<Decl*> m_depends;
        void add(Decl *decl) {
            CTASSERTF(decl != nullptr, "decl is null");
            if (auto it = std::find(m_depends.begin(), m_depends.end(), decl); it == m_depends.end())
                m_depends.push_back(decl);
        }
    };

    using declmap_t = std::map<std::string, Decl*>;

    declmap_t get_builtin_types();

    class Sema {
        Sema *m_parent;
        ResolveStack *m_resolve;
        logger_t *m_logger;
        declmap_t m_decls;

        std::string m_namespace;

        std::vector<std::string> imports;

    public:
        Sema(const Sema&) = delete;
        Sema& operator=(const Sema&) = delete;

        const std::string& get_namespace() const { return m_namespace; }

        Sema(Sema *parent)
            : m_parent(parent)
            , m_resolve(parent->m_resolve)
            , m_logger(parent->m_logger)
        { }

        Sema(logger_t *logger)
            : m_parent(nullptr)
            , m_resolve(new ResolveStack())
            , m_logger(logger)
            , m_decls(get_builtin_types())
        { }

        void add_decl(const char *name, Decl *decl);
        Decl *get_decl(const char *name) const;

        void forward_module(ref_ast_t *mod);
        void resolve_all();

        Type *resolve_type(ref_ast_t *ast);
        Type *resolve_generic(ref_ast_t *ast);

        void emit_all(io_t *source, io_t *header, const char *file);

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
    public:
        Type(const node_t *node, TreeKind kind, const char *name)
            : Decl(node, kind, name)
        { }

        virtual std::string get_cxx_name(const char *) const { return ""; }

        bool is_type() const override { return true; }
        virtual const char *get_opaque_name() const { return nullptr; }
    };

    class OpaqueType : public Type {
    public:
        OpaqueType(const node_t *node, const char *name)
            : Type(node, eKindTypeOpaque, name)
        { CTASSERT(name != nullptr); }

        void resolve(Sema&) override { finish_resolve(); }

        std::string get_cxx_name(const char *name) const override {
            return (name == nullptr) ? get_name() : std::format("{} {}", get_name(), name);
        }

        const char *get_opaque_name() const override { return get_name(); }
    };

    class GenericType : public Type {
    public:
        GenericType(ref_ast_t *ast)
            : Type(ast->node, eKindTypeTemplate, ast->name)
        { }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class PointerType : public Type {
        Type *m_pointee = nullptr;
    public:
        PointerType(const node_t *node, Type *pointee)
            : Type(node, eKindTypePointer, nullptr)
            , m_pointee(pointee)
        { }

        void resolve(Sema&) override { finish_resolve(); }

        std::string get_cxx_name(const char *name) const override {
            auto inner =  m_pointee->get_cxx_name(nullptr);
            return (name == nullptr) ? std::format("{}*", inner)
                                     : std::format("{} *{}", inner, name);
        }
    };

    class TemplateInstance;

    class TemplateType : public Type {
        std::vector<std::string> m_params;

    protected:
        void add_param(std::string param) { m_params.push_back(std::move(param)); }

    public:
        TemplateType(const node_t *node, const char *name)
            : Type(node, eKindTypeTemplate, name)
        { }

        std::span<const std::string> get_params() const { return m_params; }

        void resolve(Sema&) override { finish_resolve(); }

        virtual Type *instantiate(Sema& sema, std::span<Type*> args) const = 0;
    };

    class AtomicType : public Type {
        const Type *m_underlying = nullptr;

    public:
        AtomicType(const node_t *node, const Type *underlying)
            : Type(node, underlying->get_kind(), underlying->get_name())
            , m_underlying(underlying)
        { }

        void resolve(Sema&) override { finish_resolve(); }

        std::string get_cxx_name(const char *name) const override {
            auto inner = std::format("std::atomic<{}>", m_underlying->get_cxx_name(nullptr));
            return (name == nullptr) ? inner : std::format("{} {}", inner, name);
        }
    };

    class TemplateAtomic : public TemplateType {
        void init() { add_param("T"); }
    public:
        TemplateAtomic(const char *name)
            : TemplateType(node_builtin(), name)
        { init(); }

        void resolve(Sema&) override { finish_resolve(); }

        Type *instantiate(Sema&, std::span<Type*> args) const override {
            CTASSERT(args.size() == 1);
            return new AtomicType(get_node(), args[0]);
        }
    };

    class TemplateInstance : public Type {
    public:
        TemplateInstance(ref_ast_t *ast)
            : Type(ast->node, eKindTypeInstance, ast->name)
        { }
    };

    class IntType : public Type {
        digit_t m_digit;
        sign_t m_sign;

    public:
        IntType(const char *name, digit_t digit, sign_t sign)
            : Type(node_builtin(), eKindTypeInt, name)
            , m_digit(digit)
            , m_sign(sign)
        { }

        void resolve(Sema&) override { finish_resolve(); }

        std::string get_cxx_name(const char *name) const override;

        digit_t get_digit() const { return m_digit; }
        sign_t get_sign() const { return m_sign; }
    };

    class FloatType : public Type {
    public:
        FloatType(const char *name)
            : Type(node_builtin(), eKindTypeFloat, name)
        { }

        std::string get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "float" : std::format("float {}", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class BoolType : public Type {
    public:
        BoolType(const char *name)
            : Type(node_builtin(), eKindTypeBool, name)
        { }

        std::string get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "bool" : std::format("bool {}", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class StringType : public Type {
    public:
        StringType(const char *name)
            : Type(node_builtin(), eKindTypeString, name)
        { }

        void resolve(Sema&) override { finish_resolve(); }

        std::string get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "const char*" : std::format("const char *{}", name);
        }
    };

    class VoidType : public Type {
    public:
        VoidType(const char *name)
            : Type(node_builtin(), eKindTypeVoid, name)
        { }

        std::string get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "void" : std::format("void {}", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class MemoryType : public Type {
    public:
        MemoryType(const char *name)
            : Type(node_builtin(), eKindTypeMemory, name)
        { }

        std::string get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? "void*" : std::format("void *{}", name);
        }

        void resolve(Sema&) override { finish_resolve(); }
    };

    class Field : public Decl {
        ref_ast_t *m_ast = nullptr;

    public:
        Field(ref_ast_t *ast);

        void resolve(Sema& sema) override;
        void resolve_type(Sema& sema) override;

        void emit_impl(out_t& out) const override;
        void emit_field(out_t& out) const;

        ref_privacy_t get_privacy() const { return m_ast->privacy; }

        ref_ast_t *get_ast() const { return m_ast; }
    };

    class Param : public Decl {
        ref_ast_t *m_ast = nullptr;
    public:
        Param(ref_ast_t *ast)
            : Decl(ast->node, eKindParam, ast->name)
            , m_ast(ast)
        { }

        void resolve(Sema& sema) override {
            Type *type = sema.resolve_type(m_ast->type);
            if (type == nullptr) return;
            set_type(type);
            finish_resolve();
        }
    };

    class Method : public Decl {
        ref_ast_t *m_ast = nullptr;

        std::vector<Param*> m_params;

        Type *m_return = nullptr;

        bool m_thunk = false;

    public:
        Method(ref_ast_t *ast)
            : Decl(ast->node, eKindMethod, ast->name)
            , m_ast(ast)
        { }

        void resolve(Sema& sema) override;

        void emit_impl(out_t& out) const override;
        void emit_thunk(out_t& out) const;
        void emit_method(out_t& out) const;

        bool should_emit_thunk() const { return m_thunk; }

        ref_privacy_t get_privacy() const { return m_ast->privacy; }
    };

    class Case : public Decl {
        ref_ast_t *m_ast = nullptr;

    public:
        Case(ref_ast_t *ast);

        void resolve(Sema& sema) override;

        void emit_impl(out_t& out) const override;

        void emit_value(out_t& out) const;
        void emit_case(out_t& out) const;

        std::string get_value() const;
    };

    class Class final : public Type {
        ref_ast_t *m_ast = nullptr;

        Type *m_parent = nullptr;

        std::vector<Field*> m_fields;
        std::vector<Method*> m_methods;

    public:
        Class(ref_ast_t *ast);

        void resolve(Sema& sema) override;

        void emit_proto(out_t& out) const override;

        std::string get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? get_name() : std::format("{} {}", get_name(), name);
        }

        void emit_impl(out_t& out) const override;

        void emit_reflection(Sema& sema, out_t& out) const override;

        void get_deps(DeclDepends& deps) const override {
            if (m_parent != nullptr)
                deps.add(m_parent);

            for (auto field : m_fields)
                deps.add(field->get_type());
        }
    };

    class Struct final : public Type {
        ref_ast_t *m_ast = nullptr;

        std::vector<Field*> m_fields;
    public:
        Struct(ref_ast_t *ast);

        void resolve(Sema& sema) override;

        void emit_proto(out_t& out) const override;

        std::string get_cxx_name(const char *name) const override
        {
            return (name == nullptr) ? get_name() : std::format("{} {}", get_name(), name);
        }

        void emit_impl(out_t& out) const override;

        void emit_reflection(Sema& sema, out_t& out) const override;

        void get_deps(DeclDepends& deps) const override {
            for (auto field : m_fields)
                deps.add(field->get_type());
        }
    };

    class Variant : public Type {
        ref_ast_t *m_ast = nullptr;

        Type *m_underlying = nullptr;

        std::vector<Case*> m_cases;

        Case *m_default_case = nullptr;
    public:
        Variant(ref_ast_t *ast);

        std::string get_cxx_name(const char *name) const override {
            return (name == nullptr) ? get_name() : std::format("{} {}", get_name(), name);
        }

        void resolve(Sema& sema) override;

        void emit_proto(out_t& out) const override;
        void emit_impl(out_t& out) const override;
        void emit_facade(out_t& out) const;

        void emit_reflection(Sema& sema, out_t& out) const override;

        void get_deps(DeclDepends& deps) const override {
            if (m_underlying != nullptr)
                deps.add(m_underlying);
        }

        size_t max_tostring() const;

        size_t max_tostring_bitflags() const;
    };

    class Expr : public Tree {
    public:
        Type *m_type;
    };

    class Module : public Decl {
    public:
        std::vector<std::string> m_path;

        std::vector<Decl*> m_types;
    };

    void emit_module(Module *module, io_t *header, io_t *source);
}
