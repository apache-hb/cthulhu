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
        virtual void visit(struct Unary*) { }
        virtual void visit(struct Call*) { }
        virtual void visit(struct Name*) { }
        virtual void visit(struct Ternary*) { }
        virtual void visit(struct EmptyFunction*) { }
        virtual void visit(struct LinearFunction*) { }
        virtual void visit(struct BlockFunction*) { }
        virtual void visit(struct Sentinel*) { }
        virtual void visit(struct Context*) { }
    };

    struct Node {
        virtual ~Node() = default;

        virtual void visit(Visitor* it) = 0;
        virtual std::string debug() const = 0;

        size_t index;
    };

    struct Type: Node {
        virtual ~Type() = default;
    };

    struct Named: Type {
        virtual ~Named() = default;

        Named(std::string name)
            : name(name)
        { }

        std::string name;
    };

    struct Sentinel: Named {
        virtual ~Sentinel() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        Sentinel(std::string name)
            : Named(name)
        { }

        virtual std::string debug() const override {
            return fmt::format("(sentinel {})", name);
        }
    };

    struct Stmt: Node {
        virtual ~Stmt() = default;
    };

    struct Expr: Stmt {
        virtual ~Expr() = default;
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
            ADD, // expr + expr
            SUB, // expr - expr
            DIV, // expr / expr
            MUL, // expr * expr
            REM, // expr % expr

            GT, // expr > expr
            GTE, // expr >= expr
            LT, // expr < expr
            LTE, // expr <= expr
        };

        static constexpr const char* binop(Op it) {
            switch (it) {
            case ADD: return "add";
            case SUB: return "sub";
            case DIV: return "div";
            case MUL: return "mul";
            case REM: return "rem";
            case GT: return "gt";
            case GTE: return "gte";
            case LT: return "lt";
            case LTE: return "lte";
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
                binop(op), 
                lhs->debug(), 
                rhs->debug()
            );
        }

        Op op;
        Expr* lhs;
        Expr* rhs;
    };

    struct Unary: Expr {
        virtual ~Unary() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        enum Op {
            POS, // +expr
            NEG, // -expr
            REF, // &expr
            DEREF, // *expr
            NOT, // !expr
            FLIP // ~expr
        };

        static constexpr const char* unop(Op it) {
            switch (it) {
            case POS: return "pos";
            case NEG: return "neg";
            case REF: return "ref";
            case DEREF: return "deref";
            case NOT: return "not";
            case FLIP: return "flip";
            default: panic("unknown unop");
            }
        }

        virtual std::string debug() const override {
            return fmt::format("({} {})", unop(op), expr->debug());
        }

        Unary(Op op, Expr* expr)
            : op(op)
            , expr(expr)
        { }

        Op op;
        Expr* expr;
    };

    struct Call: Expr {
        virtual ~Call() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }

        Call(Expr* body, std::vector<Expr*> args)
            : body(body)
            , args(args)
        { }

        virtual std::string debug() const override {
            std::vector<std::string> strs;
            for (auto arg : args) {
                strs.push_back(arg->debug());
            }
            if (strs.empty())
                return fmt::format("({})", body->debug());
            else 
                return fmt::format("({} {})", body->debug(), fmt::join(strs, " "));
        }

        Expr* body;
        std::vector<Expr*> args;
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
            return fmt::format("(if {} {} {})", 
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

    struct Compound: Stmt {
        virtual ~Compound() = default;

        std::vector<Stmt*> stmts;
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

    struct Param {
        std::string name;
        Type* type;
    };

    using Params = std::vector<Param>;

    struct Function: Decl {
        virtual ~Function() = default;

        Function(std::string name, Params params, Type* result)
            : Decl(name)
            , params(params)
            , result(result)
        { }

        Params params;
        Type* result;
    };

    struct EmptyFunction: Function {
        virtual ~EmptyFunction() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }
        
        virtual std::string debug() const override {
            std::vector<std::string> args;
            for (auto [id, type] : params) {
                args.push_back(fmt::format("({} {})", id, type->debug()));
            }
            return fmt::format("(extern {} {} ({}))", name, result->debug(), fmt::join(args, " "));
        }

        EmptyFunction(std::string name, Params params, Type* result)
            : Function(name, params, result)
        { }
    };

    struct LinearFunction: Function {
        virtual ~LinearFunction() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }
        
        virtual std::string debug() const override {
            std::vector<std::string> args;
            for (auto [id, type] : params) {
                args.push_back(fmt::format("({} {})", id, type->debug()));
            }
            return fmt::format("(defun {} {} ({}) {})", name, result->debug(), fmt::join(args, " "), body->debug());
        }

        LinearFunction(std::string name, Params params, Type* result, Expr* body)
            : Function(name, params, result)
            , body(body)
        { }

        Expr* body;
    };

    struct BlockFunction: Function {
        virtual ~BlockFunction() = default;
        virtual void visit(Visitor* it) override { it->visit(this); }
        
        virtual std::string debug() const override {
            std::vector<std::string> args;
            for (auto [id, type] : params) {
                args.push_back(fmt::format("({} {})", id, type->debug()));
            }
            return fmt::format("(defun {} {} ({}) {})", name, result->debug(), fmt::join(args, " "), body->debug());
        }

        BlockFunction(std::string name, Params params, Type* result, Expr* body)
            : Function(name, params, result)
            , body(body)
        { }

        Expr* body;
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
