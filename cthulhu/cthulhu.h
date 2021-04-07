#pragma once

#include <peglib.h>
#include <fmt/core.h>

namespace cthulhu {
    template<typename T = std::runtime_error, typename... A>
    [[noreturn]] void panic(const char* fmt, const A&... args) {
        throw T(fmt::format(fmt, args...));
    }

    struct Context;
    struct NamedType;

    using TypeName = std::string;
    using TypeSize = size_t;

    // all types inherit from this type
    struct Node {
        virtual ~Node() = default;

        /* make sure this type is semantically valid */
        virtual void sema(Context* ctx) const = 0;

        /* chase this type to get its nested name, used for recursion checks */
        virtual const NamedType* chase(Context* ctx) const = 0;

        /* get the name of this node */
        virtual TypeName str() const = 0;

        /* is this type is fully resolved */
        virtual bool resolved() const { return true; }

        /* is a unit type */
        virtual bool unit() const { return false; }
    };

    using Type = Node;

    struct PointerType : Node {
        virtual ~PointerType() = default;
        virtual void sema(Context* ctx) const override;
        virtual TypeName str() const override { return "*" + type->str(); }
        virtual const NamedType* chase(Context* ctx) const override { return type->chase(ctx); }

        Type* type;

        PointerType(Type* type)
            : type(type)
        { }
    };

    using Types = std::vector<Type*>;

    struct ClosureType : Node {
        virtual ~ClosureType() = default;
        virtual void sema(Context* ctx) const override;
        virtual TypeName str() const override { return "closure"; }
        virtual const NamedType* chase(Context*) const override {
            panic("chasing closure type");
        }

        Types args;
        Type* result;

        ClosureType(Types args, Type* result)
            : args(args)
            , result(result)
        { }
    };

    struct NamedType : Node {
        virtual ~NamedType() = default;
        virtual const NamedType* chase(Context*) const override { return this; }
        virtual TypeName str() const override { return name; }

        TypeName name;

        NamedType(TypeName name) : name(name) { }
    };

    // a type that has yet to be resolved
    struct SentinelType : NamedType { 
        virtual ~SentinelType() = default;
        virtual void sema(Context*) const override;
        virtual bool resolved() const override { return false; }

        SentinelType(TypeName name)
            : NamedType(name)
        { }
    };

    struct IntType : NamedType {
        virtual ~IntType() = default;
        virtual void sema(Context*) const override { }

        IntType(TypeName name)
            : NamedType(name)
        { }
    };

    struct VoidType : NamedType {
        virtual ~VoidType() = default;
        virtual void sema(Context*) const override { }
        virtual bool unit() const { return true; }

        VoidType() 
            : NamedType("void")
        { }
    };

    struct BoolType : NamedType {
        virtual ~BoolType() = default;
        virtual void sema(Context*) const override { }

        BoolType()
            : NamedType("bool")
        { }
    };

    // an alias of one type to another type
    struct AliasType : NamedType {
        virtual ~AliasType() = default;
        virtual void sema(Context* ctx) const override;
        virtual bool unit() const { return type->unit(); }

        Type* type;

        AliasType(TypeName name, Type* type)
            : NamedType(name)
            , type(type)
        { }
    };

    struct Field {
        std::string name;
        Type* type;
    };

    struct TypeFields : std::vector<Field> {
        void add(const Field& field);
        void sema(Context* ctx) const;
    };

    // a record composed of other types
    struct RecordType : NamedType { 
        virtual ~RecordType() = default;
        virtual void sema(Context* ctx) const override;

        TypeFields fields;

        RecordType(TypeName name, TypeFields fields)
            : NamedType(name)
            , fields(fields)
        { }
    };

    struct UnionType : NamedType {
        virtual ~UnionType() = default;
        virtual void sema(Context* ctx) const override;

        TypeFields fields;

        UnionType(TypeName name, TypeFields fields)
            : NamedType(name)
            , fields(fields)
        { }
    };

    struct VariantCase {
        TypeName name;
        TypeFields fields;
    };

    struct VariantCases : std::vector<VariantCase> {
        void add(const VariantCase& item);
    };

    struct VariantType : NamedType {
        virtual ~VariantType() = default;
        virtual void sema(Context* ctx) const override;

        Type* parent;
        VariantCases cases;

        VariantType(TypeName name, Type* parent, VariantCases cases)
            : NamedType(name)
            , parent(parent)
            , cases(cases)
        { }
    };

    struct Stmt {
        virtual ~Stmt() = default;
    };

    struct Return : Stmt {
        virtual ~Return() = default;
    };

    struct FunctionType : NamedType {
        virtual ~FunctionType() = default;
        virtual void sema(Context* ctx) const override;

        TypeFields args;
        Type* result;
        Stmt* body;

        FunctionType(TypeName name, TypeFields args, Type* result, Stmt* body)
            : NamedType(name)
            , args(args)
            , result(result)
            , body(body)
        { }
    };

    namespace target {
        // size of a pointer
        constexpr TypeSize PTR = 8;

        // default size of a boolean
        constexpr TypeSize B = 1;

        extern IntType* CHAR;
        extern IntType* SHORT;
        extern IntType* INT;
        extern IntType* LONG;
        extern IntType* SIZE;
        extern IntType* UCHAR;
        extern IntType* USHORT;
        extern IntType* UINT;
        extern IntType* ULONG;
        extern IntType* USIZE;
        extern BoolType* BOOL;
        extern VoidType* VOID;

        extern NamedType* VARIANT;

        extern std::vector<NamedType*> BUILTINS;
    }

    // init the global compiler state
    void init();

    struct Context {
        // create a new compilation unit
        Context(std::string source);

        // the source text of this compilation unit
        std::string text;
        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;

        void parse();

        // perform semantic analysis
        // and generate IR
        void sema();

        // emit IR as assembly
        void emit();

        // resolve all types in the current compilation unit
        // also performs checks for recursion
        FunctionType* function(std::shared_ptr<peg::Ast> ast);
        Stmt* body(std::shared_ptr<peg::Ast> ast);
        RecordType* record(std::shared_ptr<peg::Ast> ast);
        UnionType* union_(std::shared_ptr<peg::Ast> ast);
        AliasType* alias(std::shared_ptr<peg::Ast> ast);
        VariantType* variant(std::shared_ptr<peg::Ast> ast);

        VariantCase vcase(std::shared_ptr<peg::Ast> ast);
        Field field(std::shared_ptr<peg::Ast> ast);
        Type* type(std::shared_ptr<peg::Ast> ast);

        // register a named type
        // if a sentinal type with the same name is found
        // then replace that type
        void add(NamedType* other);

        // find a type
        NamedType* get(const TypeName& name);

        // all user defined types in the current compilation unit
        std::vector<NamedType*> types;

        template<typename F>
        void enter(const Type* type, bool allow, bool opaque, F&& func) {
            push(type, allow, opaque);
            func();
            pop();
        }

        // @param type: the type being pushed
        // @param allow: does this type allow itself to be nested
        // @param opaque: is the type being pushed an opaque type
        void push(const Type* type, bool allow, bool opauqe);
        void pop();

        struct Frame {
            const Type* type;
            bool nesting;
        };

        // used for detecting recursive types
        std::vector<Frame> stack;
    };
}
