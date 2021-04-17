#pragma once

#include <peglib.h>
#include <fmt/format.h>

namespace ctu {
    template<typename T = std::runtime_error, typename... A>
    [[noreturn]] void panic(const char* fmt, const A&... args) {
        throw T(fmt::format(fmt, args...));
    }

    struct Visitor {
        virtual ~Visitor() = default;
        virtual void visit(struct Literal*) { }
        virtual void visit(struct Binary*) { }
        virtual void visit(struct Call*) { }
        virtual void visit(struct Name*) { }
        virtual void visit(struct Ternary*) { }
        virtual void visit(struct Function*) { }
        virtual void visit(struct Context*) { }
    };

    struct Node {
        virtual ~Node() = default;

        virtual void visit(Visitor* it) = 0;

        size_t index;
    };

    struct Expr: Node {
        virtual ~Expr() = default;

        virtual std::string debug() const { 
            return "expr"; 
        } 
    };

    struct Literal: Expr {
        virtual ~Literal() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        Literal(size_t value)
            : value(value)
        { }

        virtual std::string debug() const override { 
            return fmt::format("{}", value); 
        }

        size_t value;
    };

    struct Binary: Expr {
        virtual ~Binary() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        enum Op {
            ADD,
            SUB,
            DIV,
            MUL,
            REM
        };

        static constexpr const char* binop(Op it) {
            switch (it) {
            case ADD: return "add";
            case SUB: return "sub";
            case DIV: return "div";
            case MUL: return "mul";
            case REM: return "rem";
            default: panic("unknown binop");
            }
        }

        Binary(Op op, Expr* lhs, Expr* rhs)
            : op(op)
            , lhs(lhs)
            , rhs(rhs)
        { }

        virtual std::string debug() const override {
            return fmt::format("({} {} {})", 
                lhs->debug(), 
                binop(op), 
                rhs->debug()
            );
        }

        Op op;
        Expr* lhs;
        Expr* rhs;
    };

    struct Call: Expr {
        virtual ~Call() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        Call(Expr* body)
            : body(body)
        { }

        virtual std::string debug() const override {
            return fmt::format("({}())", body->debug());
        }

        Expr* body;
    };

    struct Name: Expr {
        virtual ~Name() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        Name(std::string name)
            : name(name)
        { }

        virtual std::string debug() const override {
            return name;
        }

        std::string name;
    };

    struct Ternary: Expr {
        virtual ~Ternary() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        virtual std::string debug() const override {
            return fmt::format("({} ? {} : {})", 
                cond->debug(),
                yes->debug(),
                no->debug()
            );
        }

        Ternary(Expr* cond, Expr* yes, Expr* no)
            : cond(cond)
            , yes(yes)
            , no(no)
        { }

        Expr* cond;
        Expr* yes;
        Expr* no;
    };

    struct Symbol: Node {
        virtual ~Symbol() = default;

    protected:
        Symbol(std::string name)
            : name(name)
        { }

    public:
        std::string name;
    };

    struct Decl: Symbol {
        virtual ~Decl() = default;

    protected:
        Decl(std::string name)
            : Symbol(name)
        { }
    };

    struct Function: Decl {
        virtual ~Function() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        Function(std::string name, Expr* expr)
            : Decl(name)
            , expr(expr)
        { }

        Expr* expr;
    };

    template<typename T>
    struct Symbols: std::vector<Symbol*> {
        void add(T* it) {
            for (auto& symbol : *this) {
                if (symbol->name == it->name) {
                    panic("multiple definitions of `{}`", it->name);
                }
            }

            push_back(it);
        }

        T* get(const std::string& name) {
            for (auto symbol : *this) {
                if (symbol->name == name) {
                    return (T*)symbol;
                }
            }

            panic("failed to find symbol `{}`", name);
        }
    };

    struct Context {
        void add(Decl* decl) {
            globals.add(decl);
        }

        size_t find(std::string name) {
            return globals.get(name)->index;
        }

        void visit(Visitor* it) { it->visit(this); }

        Symbols<Decl> globals;
    };

    void init();
    Context parse(std::string source);
}
