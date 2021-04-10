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
        struct NamedType;

        // basic nodes
        struct Node: std::enable_shared_from_this<Node> {
            virtual ~Node() = default;

            /* visit this node */
            virtual void visit(std::shared_ptr<Visitor> visitor) const = 0;

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
        };

        struct Stmt: Node {
            virtual ~Stmt() = default;
        };

        struct Decl: Stmt {
            virtual ~Decl() = default;
        };

        struct Expr: Stmt {
            virtual ~Expr() = default;
        };

        // types

        struct PointerType: Type {
            virtual ~PointerType() = default;

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            PointerType(std::shared_ptr<Type> type)
                : type(type)
            { }

        private:
            std::shared_ptr<Type> type;
        };

        struct ArrayType: Type {
            virtual ~ArrayType() = default;

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

            virtual bool resolved() const { return true; }

            NamedType(std::string name)
                : name(name)
            { }

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

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            AliasType(std::string name, std::shared_ptr<Type> type)
                : NamedType(name)
                , type(type)
            { }

        private:
            std::shared_ptr<Type> type;
        };

        struct SentinelType: NamedType {
            virtual ~SentinelType() = default;

            virtual bool resolved() const { return false; }

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

            SentinelType(std::string name)
                : NamedType(name)
            { }
        };

        struct BuiltinType: NamedType {
            virtual ~BuiltinType() = default;

            BuiltinType(std::string name)
                : NamedType(name)
            { }
        };

        struct ScalarType: BuiltinType {
            virtual ~ScalarType() = default;

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

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

        private:
            UnaryOp op;
            std::shared_ptr<Expr> expr;
        };

        enum struct BinaryOp {
            ADD, // expr + expr
            SUB, // expr - expr
            DIV, // expr / expr
            MOD, // expr % expr
            MUL, // expr * expr
            AND, // expr && expr
            OR, // expr || expr
            XOR, // expr ^ expr
            BITAND, // expr & expr
            BITOR, // expr | expr
            SHL, // expr << expr
            SHR, // expr >> expr
            GT, // expr > expr
            GTE, // expr >= expr
            LT, // expr < expr
            LTE // expr <= expr
        };

        struct Binary: Expr {
            virtual ~Binary() = default;

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

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
        };

        struct IntLiteral: Literal {
            virtual ~IntLiteral() = default;

            virtual void visit(std::shared_ptr<Visitor> visitor) const override;

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
        virtual void visit(const std::shared_ptr<const ast::AliasType> node) = 0;
        virtual void visit(const std::shared_ptr<const ast::SentinelType> node) = 0;
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

        struct Frame {
            std::shared_ptr<ast::Type> type;
            bool nesting;
        };

        template<typename F>
        void enter(std::shared_ptr<ast::Type> type, bool nesting, bool opaque, F&& func) {
            for (const auto& frame : stack) {
                if (frame.type == type) {
                    if (opaque && frame.nesting)
                        break;

                    panic("recursive type detected");
                }
            }
            
            stack.push_back({ type, nesting });
            func();
            stack.pop_back();
        }

    protected:
        // semantic validation stack
        std::vector<std::shared_ptr<ast::Type>> stack;

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
        void buildAlias(Context* ctx, std::shared_ptr<peg::Ast> ast);

        ast::Fields buildFields(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::Type> buildType(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::ArrayType> buildArray(Context* ctx, std::shared_ptr<peg::Ast> ast);
    
        std::shared_ptr<ast::Expr> buildExpr(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::Binary> buildBinary(Context* ctx, std::shared_ptr<peg::Ast> ast);
        std::shared_ptr<ast::IntLiteral> buildNumber(Context* ctx, std::shared_ptr<peg::Ast> ast);
    };
}
