#pragma once

#include <string>
#include <vector>

namespace ctu::ast {
    struct Context;

    struct Node {
        virtual ~Node() = default;
    };

    // all the type related stuff

    struct Type: Node {
        virtual ~Type() = default;
        virtual Type* resolve(Context*) { return this; }
    };

    // all symbols have an associated type
    struct Symbol: Type {
        virtual ~Symbol() = default;

        Symbol(std::string name)
            : name(name)
        { }

        const std::string name;
    };

    struct Builtin: Symbol {
        virtual ~Builtin() = default;

        Builtin(std::string name)
            : Symbol(name)
        { }
    };

    struct Field {
        std::string name;
        Type* type;
    };

    using Fields = std::vector<Field>;

    struct Record: Symbol {
        virtual ~Record() = default;
        virtual Type* resolve(Context* ctx) override;

        Record(std::string name, Fields fields)
            : Symbol(name)
            , fields(fields)
        { }

    private:
        Fields fields;
    };

    struct Sentinel: Symbol {
        virtual ~Sentinel() = default;
        virtual Type* resolve(Context* ctx) override;
    };

    struct Pointer: Type {
        virtual ~Pointer() = default;
        virtual Type* resolve(Context* ctx) override;

        Pointer(Type* type)
            : type(type)
        { }

    private:
        Type* type;
    };

    struct Param {
        std::string name;
        Type* type;
    };

    using Params = std::vector<Param>;

    struct Function: Symbol {
        virtual ~Function() = default;
        virtual Type* resolve(Context* ctx) override;

        Function(std::string name, Params params, Type* result)
            : Symbol(name)
            , params(params)
            , result(result)
        { }

        Params params;
        Type* result;
    };

    struct Scope {
        void define(Symbol* symbol);
        Symbol* get(std::string name);

        std::vector<Symbol*> symbols;
    };

    struct Context {
        void define(Symbol* symbol);
        Symbol* get(std::string name);

        void push();
        void pop();

        std::vector<Scope> scopes;
    };
}
