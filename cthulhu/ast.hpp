#pragma once

#include "token.hpp"

namespace cthulhu::ast {
    struct Printer {
        void write(const utf8::string& text) {
            buffer += utf8::string(depth * 2, ' ') + text + "\n";
        }

        template<typename F>
        void section(const utf8::string& name, F&& func) {
            write(name);
            depth++;
            func();
            depth--;
        }
    
        utf8::string buffer = "";
        int depth = 0;
    };

    struct Node {
        virtual ~Node() { }
        virtual bool equals(const ptr<Node>) const { throw std::runtime_error("unimplemented"); }
        virtual void visit(Printer*) const { throw std::runtime_error("unimplemented"); }
    };

    struct Ident : Node {
        Ident(Token ident);

        virtual ~Ident() override { }

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->write("- ident `" + *token.ident() + "`");
        }

    private:
        Token token;
    };

    struct Type : Node { };
    struct Stmt : Node { };
    struct Expr : Stmt { };
    struct Decl : Node { };


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
        enum UnaryOp {
            INVALID,
            NOT,
            FLIP,
            POS,
            NEG,
            DEREF,
            REF
        };

        UnaryExpr(UnaryOp op, ptr<Expr> expr);
        virtual ~UnaryExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section(utf8::string("- unary ") + str(), [&] {
                expr->visit(out);
            });
        }

    private:
        const char* str() const {
            switch (op) {
            case NOT: return "NOT";
            case FLIP: return "FLIP";
            case POS: return "POS";
            case NEG: return "NEG";
            case DEREF: return "DEREF";
            case REF: return "REF";
            default: return "INVALID";
            }
        }

        UnaryOp op;
        ptr<Expr> expr;
    };

    struct BinaryExpr : Expr {
        enum BinaryOp {
            INVALID,
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

        BinaryExpr(BinaryOp op, ptr<Expr> lhs, ptr<Expr> rhs);
        virtual ~BinaryExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section(utf8::string("- binary `") + str() + "`", [&] {
                lhs->visit(out);
                rhs->visit(out);
            });
        }

    private:
        const char* str() const {
            switch (op) {
            case ADD: return "ADD";
            case SUB: return "SUB";
            case MUL: return "MUL";
            case MOD: return "MOD";
            case DIV: return "DIV";
            case BITAND: return "BITAND";
            case BITOR: return "BITOR";
            case XOR: return "XOR";
            case AND: return "AND";
            case OR: return "OR";
            case SHL: return "SHL";
            case SHR: return "SHR";
            case LT: return "LT";
            case LTE: return "LTE";
            case GT: return "GT";
            case GTE: return "GTE";
            case EQ: return "EQ";
            case NEQ: return "NEQ";
            default: return "INVALID";
            }
        }

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

        virtual void visit(Printer* out) const override {
            out->write("- int `" + std::to_string(number.number) + "`");
        }

    private:
        Number number;
    };

    struct BoolExpr : Expr {
        BoolExpr(bool val);
        virtual ~BoolExpr() override { }

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->write(val ? "- bool `true`" : "- bool `false`");
        }

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



    //
    // toplevel declarations
    //

    struct Attribute : Decl {
        Attribute(vec<ptr<Ident>> path, vec<ptr<CallArg>> args = vec<ptr<CallArg>>());
    private:
        vec<ptr<Ident>> path;
        vec<ptr<CallArg>> args;
    };

    struct Attributes : Decl {
        Attributes(vec<ptr<Attribute>> attributes, ptr<Decl> decl);
    private:
        vec<ptr<Attribute>> attributes;
        ptr<Decl> decl;
    };

    struct Alias : Decl {
        Alias(ptr<Ident> name, ptr<Type> type);
    private:
        ptr<Ident> name;
        ptr<Type> type;
    };

    struct Record : Decl {

    };

    struct Union : Decl {

    };

    struct Enum : Decl {

    };

    struct Variant : Decl {

    };

    struct Import : Node {
        Import(vec<ptr<Ident>> path, bool wildcard, vec<ptr<Ident>> items = vec<ptr<Ident>>());

        virtual void visit(Printer* out) const override {
            out->section("- import", [&] {
                out->section("- path", [&] {
                    for (auto it : path) 
                        it->visit(out);
                });

                if (wildcard) {
                    out->write("- wildcard");
                } else if (items.empty()) {
                    out->section("- items", [&] {
                        out->write("...");
                    });
                } else {
                    out->section("- items", [&] {
                        for (auto it : items)
                            it->visit(out);
                    });
                }
            });
        }

    private:
        vec<ptr<Ident>> path;
        vec<ptr<Ident>> items;
        bool wildcard;
    };

    struct Unit : Node {
        Unit(vec<ptr<Import>> imports, vec<ptr<Decl>> decls);
        
        virtual void visit(Printer* out) const override {
            out->section("- unit", [&] {
                out->section("- imports `" + std::to_string(imports.size()) + "`", [&] {
                    for (auto it : imports) 
                        it->visit(out);
                });
                out->section("- decls `" + std::to_string(decls.size()) + "`", [&] {
                    for (auto it : decls)
                        it->visit(out);
                });
            });
        }

    private:
        vec<ptr<Import>> imports;
        vec<ptr<Decl>> decls;
    };
}
