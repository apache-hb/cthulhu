#pragma once

#include <peglib.h>

namespace cthulhu {
    struct Context;

    // init the global compiler state
    void init();

    using TypeName = std::string;
    using TypeSize = size_t;

    namespace target {
        constexpr TypeSize PTR = 8;
    }

    // all types inherit from this type
    struct Type {
        virtual ~Type() = default;
        virtual TypeSize size(Context* ctx) const = 0;
        virtual bool resolved() const { return true; }
    };

    struct PointerType : Type {
        virtual ~PointerType() = default;
        virtual TypeSize size(Context* ctx) const override;

        Type* type;

        PointerType(Type* type)
            : type(type)
        { }
    };

    struct NamedType : Type {
        virtual ~NamedType() = default;
        TypeName name;

        NamedType(TypeName name) : name(name) { }
    };

    // a type that has yet to be resolved
    struct SentinelType : NamedType { 
        virtual ~SentinelType() = default;
        virtual TypeSize size(Context* ctx) const override;
        virtual bool resolved() const override { return false; }

        SentinelType(TypeName name)
            : NamedType(name)
        { }
    };

    // a builtin type such as int or void
    struct BuiltinType : NamedType { 
        virtual ~BuiltinType() = default;
        virtual TypeSize size(Context* ctx) const override;

        TypeSize self;

        BuiltinType(TypeName name, TypeSize size)
            : NamedType(name)
            , self(size)
        { }
    };

    // an alias of one type to another type
    struct AliasType : NamedType {
        virtual ~AliasType() = default;
        virtual TypeSize size(Context* ctx) const override;

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

    using TypeFields = std::vector<Field>;

    // a record composed of other types
    struct RecordType : NamedType { 
        virtual ~RecordType() = default;
        virtual TypeSize size(Context* ctx) const override;

        TypeFields fields;

        RecordType(TypeName name, TypeFields fields)
            : NamedType(name)
            , fields(fields)
        { }
    };

    struct Context {
        // create a new compilation unit
        Context(std::string source);

        // the source text of this compilation unit
        std::string text;
        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;



        // resolve all types in the current compilation unit
        // also performs checks for recursion
        void resolve();
        RecordType* record(std::shared_ptr<peg::Ast> ast);
        AliasType* alias(std::shared_ptr<peg::Ast> ast);

        Field field(std::shared_ptr<peg::Ast> ast);
        Type* type(std::shared_ptr<peg::Ast> ast);


        // register a named type
        // if a sentinal type with the same name is found
        // then replace that type
        void add(NamedType* other);

        // find a type
        NamedType* get(const TypeName& name);

        // all types in the current compilation unit
        std::vector<NamedType*> types = {
            new BuiltinType("int", 4),
            new BuiltinType("void", 0)
        };



        template<typename F>
        TypeSize enter(const NamedType* type, F&& func) {
            push(type);
            TypeSize size = func();
            pop();
            return size;
        }

        void push(const NamedType* type);
        void pop();

        // used for detecting recursive types
        std::vector<const NamedType*> stack;
    };
}
