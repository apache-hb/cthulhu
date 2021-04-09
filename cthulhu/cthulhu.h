#pragma once

#include <peglib.h>
#include <fmt/core.h>

#define ASSERT(expr) if (!(expr)) { cthulhu::panic("assert[{}:{}]: {}", __FILE__, __LINE__, #expr); }

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

            /* is this type a scalar type */
            virtual bool scalar() const { return false; }
        };

        struct Stmt: Node {
            virtual ~Stmt() = default;
        };

        struct Decl: Stmt {
            virtual ~Decl() = default;
        };

        struct Expr: Stmt {
            virtual ~Expr() = default;

            /* can this expression be constant evaluated */
            virtual bool constant() const { return false; }

            virtual std::shared_ptr<Type> type(Context* ctx) const = 0;
        };

        // types

        struct PointerType: Type {
            virtual ~PointerType() = default;

            virtual void sema(Context*) const override { 
                /* pointers are always valid for now */
            }

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            PointerType(std::shared_ptr<Type> type)
                : type(type)
            { }

        private:
            std::shared_ptr<Type> type;
        };

        struct ArrayType: Type {
            virtual ~ArrayType() = default;

            virtual void sema(Context* ctx) const override { 
                type->sema(ctx);

                if (type->unit(ctx)) {
                    panic("array type may not be a unit type");
                }

                if (size) {
                    size->sema(ctx);
                    
                    if (!size->type(ctx)->scalar() && size->constant()) {
                        panic("array size must evaluate to a constant scalar");
                    }
                }
            }

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            ArrayType(std::shared_ptr<Type> type, std::shared_ptr<Expr> size)
                : type(type)
                , size(size)
            { }

        private:
            std::shared_ptr<Type> type;
            std::shared_ptr<Expr> size;
        };

        struct ClosureType: Type {
            virtual ~ClosureType() = default;
        };

        struct NamedType: Type {
            virtual ~NamedType() = default;

            NamedType(std::string name)
                : name(name)
            { }

            virtual bool resolved() const { return true; }

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

                    type->sema(ctx);
                }
            }

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            RecordType(std::string name, Fields fields)
                : NamedType(name)
                , fields(fields)
            { }

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

            virtual bool resolved() const { return false; }

            virtual void visit(std::shared_ptr<Visitor>) const override {
                panic("a sentinel type `{}` was visited", name);
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

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            virtual bool scalar() const override { 
                return true; 
            }

            ScalarType(std::string name, int width, bool sign)
                : BuiltinType(name) 
                , width(width)
                , sign(sign)
            { }

        private:
            // width of the type, also the alignment
            int width;

            // true if signed, false if unsigned
            bool sign;
        };

        struct BoolType: BuiltinType {
            virtual ~BoolType() = default;

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            BoolType()
                : BuiltinType("bool") 
            { }
        };

        struct VoidType: BuiltinType {
            virtual ~VoidType() = default;

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

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

        enum struct BinaryOp {
            ADD
        };

        struct Binary: Expr {
            virtual ~Binary() = default;

            virtual bool constant() const override { 
                return lhs->constant() && rhs->constant();
            }

            virtual void sema(Context* ctx) const override { 
                if (lhs->type(ctx) != rhs->type(ctx)) {
                    panic("binary operands had mismatching types");
                }
            }

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;
            virtual std::shared_ptr<Type> type(Context* ctx) const override;

            Binary(BinaryOp op, std::shared_ptr<Expr> lhs, std::shared_ptr<Expr> rhs)
                : op(op)
                , lhs(lhs)
                , rhs(rhs)
            { }

        private:
            BinaryOp op;
            std::shared_ptr<Expr> lhs;
            std::shared_ptr<Expr> rhs;
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

            virtual void sema(Context*) const override { 
                if (!suffix.empty()) {
                    /* TODO: check that value fits into suffix type */
                }
            }

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            virtual std::shared_ptr<Type> type(Context* ctx) const override;

            IntLiteral(std::string digit, int base, std::string suffix)
                : suffix(suffix)
                , value(std::strtoull(digit.c_str(), nullptr, base))
            { 
                if (value == ULLONG_MAX && errno == ERANGE) {
                    panic("`{}` was out of range", digit);
                }
            }

        private:
            std::string suffix;
            uint64_t value;
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
    }

    struct Visitor: std::enable_shared_from_this<Visitor> {
        virtual ~Visitor() = default;

        virtual void visit(const std::shared_ptr<const ast::RecordType> node) = 0;

        virtual void visit(const std::shared_ptr<const ast::PointerType> node) = 0;
        virtual void visit(const std::shared_ptr<const ast::ArrayType> node) = 0;
        virtual void visit(const std::shared_ptr<const ast::ScalarType> node) = 0;
        virtual void visit(const std::shared_ptr<const ast::BoolType> node) = 0;
        virtual void visit(const std::shared_ptr<const ast::VoidType> node) = 0;

        virtual void visit(const std::shared_ptr<const ast::IntLiteral> node) = 0;
        virtual void visit(const std::shared_ptr<const ast::Binary> node) = 0;
    };

    struct Context {
        // add a user defined type
        void add(std::shared_ptr<ast::NamedType> type);

        // try and get a type
        // if the type isnt found then a sentinel type is added
        // which allows for order independant lookup
        std::shared_ptr<ast::NamedType> get(std::string name);

        void dbg() {
            for (auto type : types) {
                type->sema(this);
            }
        }

    protected:
        // all named types
        std::vector<std::shared_ptr<ast::NamedType>> types;

        // all builtin types
        std::vector<std::shared_ptr<ast::NamedType>> builtins;
    };

    // init the global compiler state
    void init();

    struct Builder {
        // create a new compilation unit
        Builder(std::string source);

        void build(Context* ctx);

    private:
        std::string text;
        std::shared_ptr<peg::Ast> tree;

        void buildRecord(Context* ctx, std::shared_ptr<peg::Ast> ast);

        ast::Fields buildFields(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::Type> buildType(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::ArrayType> buildArray(Context* ctx, std::shared_ptr<peg::Ast> ast);
    
        std::shared_ptr<ast::Expr> buildExpr(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::Binary> buildBinary(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::IntLiteral> buildNumber(Context* ctx, std::shared_ptr<peg::Ast> ast);
    };
}
