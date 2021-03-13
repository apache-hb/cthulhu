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
        virtual bool equals(const ptr<Node>) const { throw std::runtime_error("unimplemented equals"); }
        virtual void visit(Printer*) const { throw std::runtime_error("unimplemented visit"); }
    };

    struct Ident : Node {
        Ident(Token ident);

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
    struct Decl : Stmt { };


    ///
    /// types
    ///

    struct PointerType : Type {
        PointerType(ptr<Type> type);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Type> type;
    };

    struct ReferenceType : Type {
        ReferenceType(ptr<Type> type);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Type> type;
    };

    struct MutableType : Type {
        MutableType(ptr<Type> type);

        virtual bool equals(const ptr<Node> other) const override;
    
    private:
        ptr<Type> type;
    };

    struct ArrayType : Type {
        ArrayType(ptr<Type> type, ptr<Expr> size);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Type> type;
        ptr<Expr> size;
    };

    struct ClosureType : Type {
        ClosureType(vec<ptr<Type>> args, ptr<Type> result);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        vec<ptr<Type>> args;
        ptr<Type> result;
    };

    struct NameType : Type {
        NameType(ptr<Ident> name);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            name->visit(out);
        }

    private:
        ptr<Ident> name;
    };

    struct QualifiedType : Type {
        QualifiedType(vec<ptr<NameType>> names);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- qual `" + std::to_string(names.size()) + "`", [&] {
                for (auto name : names)
                    name->visit(out);
            });
        }
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

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Expr> cond;
        ptr<Expr> yes;
        ptr<Expr> no;
    };

    struct StringExpr : Expr {
        StringExpr(const utf8::string* string);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        const utf8::string* string;
    };

    struct IntExpr : Expr {
        IntExpr(const Number& number);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->write("- int `" + std::to_string(number.number) + "`");
        }

    private:
        Number number;
    };

    struct BoolExpr : Expr {
        BoolExpr(bool val);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->write(val ? "- bool `true`" : "- bool `false`");
        }

    private:
        bool val;
    };

    struct CharExpr : Expr {
        CharExpr(c32 letter);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        c32 letter;
    };

    struct NameExpr : Expr {
        NameExpr(ptr<QualifiedType> name);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- name", [&] {
                name->visit(out);
            });
        }
    private:
        ptr<QualifiedType> name;
    };
    
    struct CoerceExpr : Expr {
        CoerceExpr(ptr<Type> type, ptr<Expr> expr);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Type> type;
        ptr<Expr> expr;
    };

    struct SubscriptExpr : Expr {
        SubscriptExpr(ptr<Expr> expr, ptr<Expr> index);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Expr> expr;
        ptr<Expr> index;
    };

    struct AccessExpr : Expr {
        AccessExpr(ptr<Expr> body, ptr<Ident> field, bool indirect);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- access", [&] {
                body->visit(out);
                field->visit(out);
                if (indirect) {
                    out->write("- indirect");
                }
            });
        }

    private:
        ptr<Expr> body;
        ptr<Ident> field;
        bool indirect;
    };

    struct CallArg : Node {
        CallArg(ptr<Ident> name, ptr<Expr> expr);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- arg", [&] {
                if (name)
                    name->visit(out);
                expr->visit(out);
            });
        }

    private:
        ptr<Ident> name;
        ptr<Expr> expr;
    };

    struct CallExpr : Expr {
        CallExpr(ptr<Expr> func, vec<ptr<CallArg>> args);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- call", [&] {
                func->visit(out);
                out->section("- args", [&] {
                    for (auto arg : args)
                        arg->visit(out);
                });
            });
        }
    private:
        ptr<Expr> func;
        vec<ptr<CallArg>> args;
    };



    //
    // toplevel declarations
    //

    struct Alias : Decl {
        Alias(ptr<Ident> name, ptr<Type> type);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- alias", [&] {
                name->visit(out);
                type->visit(out);
            });
        }

    private:
        ptr<Ident> name;
        ptr<Type> type;
    };

    struct Field : Node {
        Field(ptr<Ident> name, ptr<Type> type);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- field", [&] {
                name->visit(out);
                type->visit(out);
            });
        }

    private:
        ptr<Ident> name;
        ptr<Type> type;
    };

    struct VarName : Node {
        VarName(ptr<Ident> name, ptr<Type> type);

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Ident> name;
        ptr<Type> type;
    };

    struct Var : Decl {
        Var(vec<ptr<VarName>> names, ptr<Expr> init, bool mut);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- var", [&] {
                if (mut) {
                    out->write("- mutable");
                }

                for (auto field : names)
                    field->visit(out);

                init->visit(out);
            });
        }

    private:
        vec<ptr<VarName>> names;
        ptr<Expr> init;
        bool mut;
    };

    struct Record : Decl {
        Record(ptr<Ident> name, vec<ptr<Field>> fields);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- record", [&] {
                name->visit(out);
                out->section("- fields", [&] {
                    for (auto field : fields)
                        field->visit(out);
                });
            });
        }

    private:
        ptr<Ident> name;
        vec<ptr<Field>> fields;
    };

    struct Union : Decl {
        Union(ptr<Ident> name, vec<ptr<Field>> fields);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Ident> name;
        vec<ptr<Field>> fields;
    };

    struct Case : Node {
        Case(ptr<Ident> name, ptr<Expr> value, vec<ptr<Field>> fields);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Ident> name;
        ptr<Expr> value;
        vec<ptr<Field>> fields;
    };

    struct Variant : Decl {
        Variant(ptr<Ident> name, ptr<QualifiedType> parent, vec<ptr<Case>> cases);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<Ident> name;
        ptr<QualifiedType> parent;
        vec<ptr<Case>> cases;
    };

    struct Param : Node {
        Param(ptr<Ident> name, ptr<Type> type, ptr<Expr> init);
    
        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- param", [&] {
                name->visit(out);
                type->visit(out);
                if (init) {
                    init->visit(out);
                }
            });
        }
    private:
        ptr<Ident> name;
        ptr<Type> type;
        ptr<Expr> init;
    };

    struct Function : Decl {
        Function(ptr<Ident> name, vec<ptr<Param>> params, ptr<Type> result, ptr<Stmt> body);
    
        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- function", [&] {
                name->visit(out);
                for (auto param : params)
                    param->visit(out);

                if (result) {
                    result->visit(out);
                }

                if (body) {
                    body->visit(out);
                }
            });
        }
    private:
        ptr<Ident> name;
        vec<ptr<Param>> params;
        ptr<Type> result;
        ptr<Stmt> body;
    };

    struct Attribute : Node {
        Attribute(ptr<QualifiedType> name, vec<ptr<CallArg>> args);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        ptr<QualifiedType> name;
        vec<ptr<CallArg>> args;
    };

    struct Decorated : Decl {
        Decorated(vec<ptr<Attribute>> attribs, ptr<Decl> decl);

        virtual bool equals(const ptr<Node> other) const override;

    private:
        // technically the grammar states that this should be recursive
        // but thats a pain in the ass to model sanely so we flatten out
        // attributes in the parser automatically
        vec<ptr<Attribute>> attribs;
        ptr<Decl> decl;
    };

    //
    // statements
    //

    struct Compound : Stmt {
        Compound(vec<ptr<Stmt>> stmts);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- compound", [&] {
                for (auto stmt : stmts)
                    stmt->visit(out);
            });
        }

    private:
        vec<ptr<Stmt>> stmts;
    };

    struct If : Node {
        ptr<Expr> cond;
        ptr<Stmt> body;
    };

    struct Branch : Stmt {
        vec<ptr<If>> branches;
    };

    struct With : Stmt {
        ptr<Expr> init;
        ptr<Stmt> body;
    };

    struct While : Stmt {
        While(ptr<Expr> cond, ptr<Stmt> body, ptr<Stmt> other);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- while", [&] {
                cond->visit(out);
                body->visit(out);
                if (other)
                    other->visit(out);
            });
        }
    private:
        ptr<Expr> cond;
        ptr<Stmt> body;
        ptr<Stmt> other;
    };

    struct Assign : Stmt {
        enum Op {
            INVALID,
            ASSIGN,
            ADDEQ,
            SUBEQ,
            DIVEQ,
            MODEQ,
            MULEQ,
            SHLEQ,
            SHREQ,
            XOREQ,
            OREQ,
            ANDEQ
        };

        Assign(Op op, ptr<Expr> body, ptr<Expr> value);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- assign", [&] {
                out->write(str());
                body->visit(out);
                value->visit(out);
            });
        }

    private:
        const char* str() const {
            switch (op) {
            case ASSIGN: return "ASSIGN";
            case ADDEQ: return "ADDEQ";
            case SUBEQ: return "SUBEQ";
            case DIVEQ: return "DIVEQ";
            case MODEQ: return "MODEQ";
            case SHLEQ: return "SHLEQ";
            case SHREQ: return "SHREQ";
            case XOREQ: return "XOREQ";
            case OREQ: return "OREQ";
            case ANDEQ: return "ANDEQ";
            default: return "INVALID";
            }
        }
        Op op;
        ptr<Expr> body;
        ptr<Expr> value;
    };

    struct Return : Stmt {
        Return(ptr<Expr> expr);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->section("- return", [&] {
                if (expr)
                    expr->visit(out);
            });
        }
    private:
        ptr<Expr> expr;
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
