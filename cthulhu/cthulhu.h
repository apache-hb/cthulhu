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
        // basic nodes
        struct Node: std::enable_shared_from_this<Node> {
            virtual ~Node() = default;

            /* visit this node */
            virtual void visit(std::shared_ptr<Visitor> visitor) const = 0;

            /* ensure node is correct */
            virtual void sema(Context* ctx) const = 0;

            template<typename T>
            std::shared_ptr<const T> as() const {
                return std::dynamic_pointer_cast<const T>(shared_from_this());
            }

            template<typename T>
            bool is() const {
                return as<T>() != nullptr;
            }
        };

        struct Type: Node {
            virtual ~Type() = default;

            /* types resolve to themselves, aside from aliases and sentinels */
            virtual std::shared_ptr<const Type> resolve(Context*) const { 
                return as<Type>();
            }

            /* is this type a void type */
            virtual bool unit(Context*) const { 
                return false; 
            }
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

            virtual std::shared_ptr<Type> type(Context* ctx) const = 0;
        };

        // types

        struct PointerType: Type {
            virtual ~PointerType() = default;
        };

        struct ArrayType: Type {
            virtual ~ArrayType() = default;
        };

        struct ClosureType: Type {
            virtual ~ClosureType() = default;
        };

        struct NamedType: Type {
            virtual ~NamedType() = default;

            NamedType(std::string name)
                : name(name)
            { }

        protected:
            std::string name;
        };

        struct Field {
            std::string name;
            std::shared_ptr<Type> type;
        };

        struct Fields: std::vector<Field> {
            void add(std::string name, std::shared_ptr<Type> type);
        };

        struct RecordType: NamedType {
            virtual ~RecordType() = default;

            virtual void sema(Context* ctx) const override { 
                for (const auto& [field, type] : fields) {
                    if (type->unit(ctx)) {
                        panic("record field `{}` was a unit type", field);
                    }
                }
            }

        private:
            Fields fields;
        };

        struct UnionType: NamedType {
            virtual ~UnionType() = default;
        };

        struct SumType: NamedType {
            virtual ~SumType() = default;
        };

        struct AliasType: NamedType {
            virtual ~AliasType() = default;

            virtual std::shared_ptr<const Type> resolve(Context*) const override { 
                panic("TODO: recursion checking `{}`", name);
            }

            virtual bool unit(Context* ctx) const override { 
                return resolve(ctx)->unit(ctx);
            }

            AliasType(std::string name, std::shared_ptr<Type> type)
                : NamedType(name)
                , type(type)
            { }

        private:
            std::shared_ptr<Type> type;
        };

        struct SentinelType: NamedType {
            virtual ~SentinelType() = default;

            virtual std::shared_ptr<const Type> resolve(Context*) const override { 
                panic("TODO: order independant lookup `{}`", name);
            }

            virtual void sema(Context*) const override {
                panic("TODO: unresolved sentinel type `{}`", name);
            }

            SentinelType(std::string name)
                : NamedType(name)
            { }
        };

        struct BuiltinType: NamedType {
            virtual ~BuiltinType() = default;

            virtual void sema(Context*) const override { 
                /* builtin types are always semantically valid */
            }

            BuiltinType(std::string name)
                : NamedType(name)
            { }
        };

        struct ScalarType: BuiltinType {
            virtual ~ScalarType() = default;

            ScalarType(std::string name)
                : BuiltinType(name) 
            { }
        };

        struct BoolType: BuiltinType {
            virtual ~BoolType() = default;

            BoolType()
                : BuiltinType("bool") 
            { }
        };

        struct VoidType: BuiltinType {
            virtual ~VoidType() = default;

            /* void is always a unit type */
            virtual bool unit(Context*) const override { 
                return true; 
            }

            VoidType()
                : BuiltinType("void") 
            { }
        };

        // expressions

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

            virtual void sema(Context* ctx) const override { 
                if (op == UnaryOp::DEREF && !expr->type(ctx)->resolve(ctx)->is<PointerType>()) {
                    panic("cannot dereference a type that isnt a pointer");
                }
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

        // top level constructs

        struct Import: Node {
            virtual ~Import() = default;
        };

        struct Unit: Node {
            virtual ~Unit() = default;

            virtual void sema(Context*) const override { 
                panic("TODO: unit semantics");
            }
        };
    }

    struct Visitor: std::enable_shared_from_this<Visitor> {
        virtual ~Visitor() = default;

        virtual void visit(const std::shared_ptr<ast::Import> node) = 0;
        virtual void visit(const std::shared_ptr<ast::Unit> node) = 0;
    };

    struct Context {
        
    };

    // init the global compiler state
    void init();

    struct Builder {
        // create a new compilation unit
        Builder(std::string source);

        std::string text;
        std::shared_ptr<peg::Ast> tree;

        std::shared_ptr<ast::Unit> build(Context* ctx);
    };
}
