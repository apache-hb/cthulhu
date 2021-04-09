#pragma once

#include <peglib.h>
#include <fmt/core.h>

namespace cthulhu {
    template<typename T = std::runtime_error, typename... A>
    [[noreturn]] void panic(const char* fmt, const A&... args) {
        throw T(fmt::format(fmt, args...));
    }

    struct Visitor;
    struct Context;

    namespace ast {
        struct Node: std::enable_shared_from_this<Node> {
            virtual ~Node() = default;

            /* visit this node */
            virtual void visit(std::shared_ptr<Visitor> visitor) const = 0;

            /* ensure node is correct */
            virtual void sema(Context* ctx) const = 0;

            template<typename T>
            std::shared_ptr<T> as() const {
                return std::dynamic_pointer_cast<T>(shared_from_this());
            }

            template<typename T>
            bool is() const {
                return as<T>() != nullptr;
            }
        };

        struct Type: Node {
            virtual ~Type() = default;
        };

        struct NamedType: Type {
            virtual ~NamedType() = default;
        };

        struct Stmt: Node {
            virtual ~Stmt() = default;
        };

        struct Decl: Stmt {
            virtual ~Decl() = default;
        };

        struct Expr: Stmt {
            virtual ~Expr() = default;

            virtual bool constant() const { return false; }

            virtual std::shared_ptr<Type> type() const = 0;
        };

        enum struct UnaryOp {
            ADD, // +expr
            SUB, // -expr
            NOT, // !expr
            FLIP, // ~expr
            REF, // &expr
            DEREF // *expr
        };

        struct Unary: Expr {
            virtual ~Unary() = default;
            virtual bool constant() const override { return expr->constant(); }

            virtual void sema(Context*) const override { 
                /* currently our rules are relaxed enough that unary operations are always semantically valid */ 
            }

        private:
            UnaryOp op;
            std::shared_ptr<Expr> expr;
        };

        struct Literal: Expr {
            virtual ~Literal() = default;
            virtual bool constant() const override { return true; }
            virtual void sema(Context*) const override { 
                /* a literal will always be semantically valid */ 
            }
        };

        struct IntLiteral: Literal {
            virtual ~IntLiteral() = default;

        private:
            int value;
        };

        struct BoolLiteral: Literal {
            virtual ~BoolLiteral() = default;

        private:
            bool value;
        };

        struct StringLiteral: Literal {
            virtual ~StringLiteral() = default;

        private:
            std::string value;
        };
    }

    struct Visitor: std::enable_shared_from_this<Visitor> {
        virtual ~Visitor() = default;

        virtual void visit(const std::shared_ptr<ast::StringLiteral> node) = 0;
    };

    // init the global compiler state
    void init();

    struct Context {
        // create a new compilation unit
        Context(std::string source);

        std::string text;
        std::shared_ptr<peg::Ast> tree;
    };
}
