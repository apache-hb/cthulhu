#pragma once

#include "token.hpp"

namespace cthulhu::ast {
    struct Node {
        virtual ~Node() { }
        virtual bool equals(const ptr<Node> other) const = 0;
    };

    struct Ident : Node {
        Ident(Token ident);

        virtual ~Ident() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        Token token;
    };

    struct Type : Node { };
    struct Expr : Node { };


    ///
    /// types
    ///

    struct PointerType : Type {
        PointerType(ptr<Type> type);
        virtual ~PointerType() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Type> type;
    };

    struct ReferenceType : Type {
        ReferenceType(ptr<Type> type);
        virtual ~ReferenceType() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Type> type;
    };

    struct MutableType : Type {
        MutableType(ptr<Type> type);
        virtual ~MutableType() override { }

        virtual bool equals(const ptr<Node> other) const override;
    
    private:
        ptr<Type> type;
    };

    struct ArrayType : Type {
        ArrayType(ptr<Type> type, ptr<Expr> size);
        virtual ~ArrayType() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Type> type;
        ptr<Expr> size;
    };

    struct ClosureType : Type {
        ClosureType(vec<ptr<Type>> args, ptr<Type> result);
        virtual ~ClosureType() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        vec<ptr<Type>> args;
        ptr<Type> result;
    };

    struct NameType : Type {
        NameType(ptr<Ident> name);
        virtual ~NameType() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Ident> name;
    };

    struct QualifiedType : Type {
        QualifiedType(vec<ptr<NameType>> names);
        virtual ~QualifiedType() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        vec<ptr<NameType>> names;
    };



    ///
    /// expressions
    ///

    struct UnaryExpr : Expr {
        UnaryExpr(Token op, ptr<Expr> expr);
        virtual ~UnaryExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

        enum UnaryOp {

        };

    private:
        UnaryOp op;
        ptr<Expr> expr;
    };

    struct BinaryExpr : Expr {
        BinaryExpr(Token op, ptr<Expr> lhs, ptr<Expr> rhs);
        virtual ~BinaryExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

        enum BinaryOp {

        };

    private:
        BinaryOp op;
        ptr<Expr> lhs;
        ptr<Expr> rhs;
    };

    struct TernaryExpr : Expr {
        TernaryExpr(ptr<Expr> cond, ptr<Expr> yes, ptr<Expr> no);
        virtual ~TernaryExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Expr> cond;
        ptr<Expr> yes;
        ptr<Expr> no;
    };

    struct StringExpr : Expr {

    };

    struct IntExpr : Expr {

    };

    struct BoolExpr : Expr {

    };

    struct CharExpr : Expr {

    };

    struct CoerceExpr : Expr {
        ptr<Type> type;
        ptr<Expr> expr;
    };

    struct SubscriptExpr : Expr {
        ptr<Expr> expr;
        ptr<Expr> index;
    };

    struct AccessExpr : Expr {
        ptr<Expr> body;
        ptr<Ident> field;
    };

    struct CallArg : Node {
        ptr<Ident> name;
        ptr<Expr> expr;
    };

    struct CallExpr : Expr {
        ptr<Expr> func;
        vec<ptr<CallArg>> args;
    };
}
