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
            NOT,
            FLIP,
            POS,
            NEG,
            DEREF,
            REF
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
            ADD,
            SUB,
            MUL,
            MOD,
            DIV,
            BITAND,
            BITOR,
            XOR,
            AND,
            OR,
            SHL,
            SHR,
            LT,
            LTE,
            GT,
            GTE,
            EQ,
            NEQ
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
        StringExpr(const utf8::string* string);
        virtual ~StringExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        const utf8::string* string;
    };

    struct IntExpr : Expr {
        IntExpr(const Number& number);
        virtual ~IntExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        Number number;
    };

    struct BoolExpr : Expr {
        BoolExpr(bool val);
        virtual ~BoolExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        bool val;
    };

    struct CharExpr : Expr {
        CharExpr(c32 letter);
        virtual ~CharExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

    private:
        c32 letter;
    };

    struct NameExpr : Expr {
        NameExpr(ptr<QualifiedType> name);
        virtual ~NameExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<QualifiedType> name;
    };
    
    struct CoerceExpr : Expr {
        CoerceExpr(ptr<Type> type, ptr<Expr> expr);
        virtual ~CoerceExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Type> type;
        ptr<Expr> expr;
    };

    struct SubscriptExpr : Expr {
        SubscriptExpr(ptr<Expr> expr, ptr<Expr> index);
        virtual ~SubscriptExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Expr> expr;
        ptr<Expr> index;
    };

    struct AccessExpr : Expr {
        AccessExpr(ptr<Expr> body, ptr<Ident> field, bool indirect);
        virtual ~AccessExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Expr> body;
        ptr<Ident> field;
        bool indirect;
    };

    struct CallArg : Node {
        CallArg(ptr<Ident> name, ptr<Expr> expr);
        virtual ~CallArg() override { }

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Ident> name;
        ptr<Expr> expr;
    };

    struct CallExpr : Expr {
        CallExpr(ptr<Expr> func, vec<ptr<CallArg>> args);
        virtual ~CallExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Expr> func;
        vec<ptr<CallArg>> args;
    };
}
