#pragma once

#include "cthulhu.h"
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
    };

    struct IntLiteral: Expr {
        virtual ~IntLiteral() = default;
        virtual std::string debug() const override;

        IntLiteral(size_t value)
            : value(value)
        { }

        size_t value;
    };

    struct BoolLiteral: Expr {
        virtual ~BoolLiteral() = default;
        virtual std::string debug() const override;

        BoolLiteral(bool value)
            : value(value)
        { }

        bool value;
    };

    struct Binary: Expr {
        virtual ~Binary() = default;
        virtual std::string debug() const override;

        enum Op {
            ADD,
            SUB,
            DIV,
            MUL,
            REM
        };

        Binary(Op op, Expr* lhs, Expr* rhs)
            : op(op)
            , lhs(lhs)
            , rhs(rhs)
        { }

        Op op;
        Expr* lhs;
        Expr* rhs;
    };

    struct Unary: Expr {
        virtual ~Unary() = default;
        virtual std::string debug() const override;

        enum Op {
            POS,
            NEG,
            REF,
            DEREF,
            NOT,
            FLIP
        };

        Unary(Op op, Expr* expr)
            : op(op)
            , expr(expr)
        { }

        Op op;
        Expr* expr;
    };

    constexpr const char* str(Unary::Op op) {
        switch (op) {
        case Unary::POS: return "pos";
        case Unary::NEG: return "neg";
        case Unary::REF: return "ref";
        case Unary::DEREF: return "deref";
        case Unary::NOT: return "not";
        case Unary::FLIP: return "flip";
        default: panic("unknown unop");
        }
    }

    constexpr const char* str(Binary::Op op) {
        switch (op) {
        case Binary::ADD: return "add";
        case Binary::SUB: return "sub";
        case Binary::DIV: return "div";
        case Binary::MUL: return "mul";
        case Binary::REM: return "rem";
        default: panic("unknown binop");
        }
    }

    using Args = std::vector<Expr*>;

    struct Call: Expr {
        virtual ~Call() = default;
        virtual std::string debug() const override;

        Call(Expr* body, Args args)
            : body(body)
            , args(args)
        { }
        
        Expr* body;
        Args args;
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

    struct SimpleFunction: Function {
        virtual ~SimpleFunction() = default;
        virtual std::string debug() const override;

        SimpleFunction(std::string name, Params params, Type* result, Expr* expr)
            : Function(name, params, result)
            , expr(expr)
        { }

        Expr* expr;
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
