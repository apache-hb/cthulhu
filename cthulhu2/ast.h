#pragma once

#include <vector>
#include <string>

namespace ctu {
    struct Context;

    struct Node {
        virtual ~Node() = default;

        virtual std::string debug() const = 0;
    };

    // all the type related stuff

    struct Type: Node {
        virtual ~Type() = default;
        virtual Type* resolve(Context*) { return this; }
    };

    struct Stmt: Node {
        virtual ~Stmt() = default;
    };

    struct Compound: Stmt {
        virtual ~Compound() = default;

        std::vector<Stmt*> stmts;
    };

    // an expression is a statement that can evaluate to a value
    struct Expr: Stmt {
        virtual ~Expr() = default;

        virtual Type* typeof(Context*) = 0;
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
        virtual Type* resolve(Context* ctx) override;
        virtual std::string debug() const override;

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
        virtual std::string debug() const override;

        Sentinel(std::string name)
            : Symbol(name)
        { }
    };

    struct Pointer: Type {
        virtual ~Pointer() = default;
        virtual Type* resolve(Context* ctx) override;
        virtual std::string debug() const override;

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
        virtual std::string debug() const override;

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

        std::vector<Scope> scopes = { {} };
    };

    Context parse(const std::string& source, std::vector<Symbol*> symbols);
}
