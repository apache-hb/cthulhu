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
        struct Node {
            virtual ~Node() = default;

            /* visit this node */
            virtual void visit(Visitor* visitor) = 0;

            virtual void sema(Context*) {
                panic("unimplemented sema");
            }

            template<typename T>
            T* as() {
                return dynamic_cast<T*>(this);
            }

            template<typename T>
            bool is() {
                return as<T>() != nullptr;
            }
        };

        struct Type: Node {
            virtual ~Type() = default;

            virtual Type* root(Context*) { return this; }
            virtual bool resolved() const { return true; }
            virtual bool unit(Context*) const { return false; }
            virtual bool scalar() const { return false; }
            virtual bool unsized() const { return false; }
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
            virtual Type* type(Context* ctx) = 0;
        };

        // stmts

        struct Compound: Stmt {
            virtual ~Compound() = default;

            std::vector<Stmt*> stmts;
        };

        // types

        struct PointerType: Type {
            virtual ~PointerType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual Type* root(Context* ctx) { return type->root(ctx); }
            virtual void sema(Context* ctx) override;

            PointerType(Type* type)
                : type(type)
            { }

            Type* type;
        };

        struct ArrayType: Type {
            virtual ~ArrayType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual Type* root(Context* ctx) { return type->root(ctx); }
            virtual bool unsized() const override { return size == nullptr; }
            virtual void sema(Context* ctx) override;

            ArrayType(Type* type, Expr* size)
                : type(type)
                , size(size)
            { }

            Type* type;
            Expr* size;
        };

        struct Types: std::vector<Type*> {
            void sema(Context* ctx);
        };

        struct ClosureType: Type {
            virtual ~ClosureType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;

            ClosureType(Types args, Type* result)
                : args(args)
                , result(result)
            { }

            Types args;
            Type* result;
        };

        struct NamedType: Type {
            virtual ~NamedType() = default;

            NamedType(std::string name)
                : name(name)
            { }

            std::string name;
        };

        struct Field {
            std::string name;
            Type* type;
        };

        struct Fields: std::vector<Field> {
            void add(std::string name, Type* type);
            void sema(Context* ctx, bool record = false);
        };

        struct RecordType: NamedType {
            virtual ~RecordType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;

            RecordType(std::string name, Fields fields)
                : NamedType(name)
                , fields(fields)
            { }

            Fields fields;
        };

        struct UnionType: NamedType {
            virtual ~UnionType() = default;
        };

        struct Case {
            std::string name;
            Fields fields;
        };

        struct Cases: std::vector<Case> {
            void add(std::string name, Fields entry);
            void sema(Context* ctx);
        };

        struct SumType: NamedType {
            virtual ~SumType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;

            SumType(std::string name, Type* parent, Cases cases)
                : NamedType(name)
                , parent(parent)
                , cases(cases)
            { }

            Type* parent;
            Cases cases;
        };

        struct AliasType: NamedType {
            virtual ~AliasType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;
            virtual bool unit(Context* ctx) const override { return type->unit(ctx); }

            AliasType(std::string name, Type* type)
                : NamedType(name)
                , type(type)
            { }

            Type* type;
        };

        struct SentinelType: NamedType {
            virtual ~SentinelType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;
            virtual bool resolved() const override { return false; }
            virtual bool unit(Context* ctx) const override;

            SentinelType(std::string name)
                : NamedType(name)
            { }
        };

        struct BuiltinType: NamedType {
            virtual ~BuiltinType() = default;

            virtual void sema(Context*) override { 
                /* builtin types should always be valid */
            }

            BuiltinType(std::string name)
                : NamedType(name)
            { }
        };

        struct ScalarType: BuiltinType {
            virtual ~ScalarType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual bool scalar() const override { return true; }

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

            virtual void visit(Visitor* visitor) override;

            BoolType()
                : BuiltinType("bool") 
            { }
        };

        struct VoidType: BuiltinType {
            virtual ~VoidType() = default;

            virtual void visit(Visitor* visitor) override;
            virtual bool unit(Context*) const override { return true; }

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

            virtual bool constant() const override { 
                return expr->constant(); 
            }

        private:
            UnaryOp op;
            Expr* expr;
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

            virtual void visit(Visitor* visitor) override;
            virtual bool constant() const override { 
                return lhs->constant() && rhs->constant(); 
            }

            virtual void sema(Context* ctx) override;
            virtual Type* type(Context* ctx) override;

            Binary(BinaryOp op, Expr* lhs, Expr* rhs)
                : op(op)
                , lhs(lhs)
                , rhs(rhs)
            { }

            BinaryOp op;
            Expr* lhs;
            Expr* rhs;
        };

        struct Literal: Expr {
            virtual ~Literal() = default;

            virtual bool constant() const override { return true; }
            virtual void sema(Context*) override { 
                /* literals should always be valid */
            }
        };

        struct IntLiteral: Literal {
            virtual ~IntLiteral() = default;

            virtual void visit(Visitor* visitor) override;

            IntLiteral(std::string digit, int base, std::string suffix)
                : suffix(suffix)
                , value(std::strtoull(digit.c_str(), nullptr, base))
            { 
                if (value == ULLONG_MAX && errno == ERANGE) {
                    panic("`{}` was out of range", digit);
                }
            }

            virtual Type* type(Context* ctx) override;
            virtual void sema(Context* ctx) override;

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

        // functions

        struct Param {
            std::string name;
            Type* type;
            Expr* init;
        };

        struct Params: std::vector<Param*> {
            void add(std::string name, Type* type, Expr* init);
        };

        struct Function: Node {
            virtual ~Function() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;

            Function(std::string name, Params params, Type* result)
                : name(name)
                , params(params)
                , result(result)
            { }

            std::string name;
            Params params;
            Type* result;
        };

        struct SimpleFunction: Function {
            virtual ~SimpleFunction() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;

            Expr* body;
        };

        struct ComplexFunction: Function {
            virtual ~ComplexFunction() = default;

            virtual void visit(Visitor* visitor) override;
            virtual void sema(Context* ctx) override;

            Compound* body;
        };

        // top level constructs

        struct Import: Node {
            virtual ~Import() = default;
        };
    }

    struct Visitor {
        virtual ~Visitor() = default;

        virtual void visit(ast::RecordType* node) = 0;
        virtual void visit(ast::SumType* node) = 0;
        virtual void visit(ast::AliasType* node) = 0;
        virtual void visit(ast::SentinelType* node) = 0;
        virtual void visit(ast::PointerType* node) = 0;
        virtual void visit(ast::ClosureType* node) = 0;
        virtual void visit(ast::ArrayType* node) = 0;
        virtual void visit(ast::ScalarType* node) = 0;
        virtual void visit(ast::BoolType* node) = 0;
        virtual void visit(ast::VoidType* node) = 0;
        virtual void visit(ast::IntLiteral* node) = 0;
        virtual void visit(ast::Binary* node) = 0;
        virtual void visit(ast::Function* node) = 0;
    };

    struct Context {
        // add a user defined type
        void add(ast::NamedType* type);

        // add a user defined function
        void add(ast::Function* func);

        // try and get a type
        // if the type isnt found then a sentinel type is added
        // which allows for order independant lookup
        ast::NamedType* get(std::string name);

        struct Frame {
            ast::Type* type;
            bool nesting;
        };

        // used for resolving types safely to prevent stack overflows
        // `type` is the type that generated this frame
        // `nesting` is true when this type can nest inside itself
        // `opaque` is true when uhhhh
        //          TODO: what does opaque do?
        template<typename F>
        void enter(ast::Type* type, bool nesting, bool opaque, F&& func) {
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

        // semantic validation stack
        std::vector<Frame> stack;

        // all named types
        std::vector<ast::NamedType*> types;

        // all builtin types
        std::vector<ast::NamedType*> builtins;

        // all functions
        std::vector<ast::Function*> funcs;
    };

    // init the global compiler state
    void init();

    struct Builder {
        // create a new compilation unit
        Builder(std::string source);

        // parse in a source file
        // and semantically validate it
        // `ctx` will contain a valid program ast
        void build(Context* ctx);

    private:
        std::string text;
        std::shared_ptr<peg::Ast> tree;

        void buildRecord(Context* ctx, std::shared_ptr<peg::Ast> ast);
        void buildAlias(Context* ctx, std::shared_ptr<peg::Ast> ast);
        void buildVariant(Context* ctx, std::shared_ptr<peg::Ast> ast);
        void buildFunction(Context* ctx, std::shared_ptr<peg::Ast> ast);

        ast::Cases buildCases(Context* ctx, std::shared_ptr<peg::Ast> ast);
        ast::Fields buildFields(Context* ctx, std::shared_ptr<peg::Ast> ast);
        ast::Type* buildType(Context* ctx, std::shared_ptr<peg::Ast> ast);
        ast::ArrayType* buildArray(Context* ctx, std::shared_ptr<peg::Ast> ast);
        ast::ClosureType* buildClosure(Context* ctx, std::shared_ptr<peg::Ast> ast);

        ast::Expr* buildExpr(Context* ctx, std::shared_ptr<peg::Ast> ast);
        ast::Binary* buildBinary(Context* ctx, std::shared_ptr<peg::Ast> ast);
        ast::IntLiteral* buildNumber(Context* ctx, std::shared_ptr<peg::Ast> ast);
    };
}
