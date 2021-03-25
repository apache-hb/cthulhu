#pragma once

#include "token.hpp"

namespace cthulhu::ast {
    struct Printer {
        void write(const str& text) {
            buffer += str(depth * 2, ' ') + text;
            if (feed) {
                buffer += "\n";
            }
        }

        template<typename F>
        void section(const str& name, F&& func) {
            write(name);
            depth++;
            func();
            depth--;
        }
    
        bool feed = true;
        str buffer = "";
        int depth = 0;
    };

    struct Node {
        virtual ~Node() { }
        virtual bool equals(const ptr<Node>) const { throw std::runtime_error("unimplemented equals"); }
        virtual void visit(Printer*) const { throw std::runtime_error("unimplemented visit"); }
        virtual void emit(Printer*) const { throw std::runtime_error("unimplemented emit"); }
    };

    struct Ident : Node {
        Ident(Token ident);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->write("- ident `" + *token.ident() + "`");
        }

        virtual void emit(Printer* out) const override {
            out->write(*token.ident());
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

        virtual void emit(Printer* out) const override {
            type->emit(out);
            out->write("*");
        }

    private:
        ptr<Type> type;
    };

    struct MutableType : Type {
        MutableType(ptr<Type> type);

        virtual bool equals(const ptr<Node> other) const override;
    
        virtual void emit(Printer* out) const override {
            // dont care about const for now
            type->emit(out);
        }

    private:
        ptr<Type> type;
    };

    struct ArrayType : Type {
        ArrayType(ptr<Type> type, ptr<Expr> size);

        virtual bool equals(const ptr<Node> other) const override;

        // TODO: dont ignore array sizes
        virtual void emit(Printer* out) const override {
            type->emit(out);
            out->write("*");
        }

    private:
        ptr<Type> type;
        ptr<Expr> size;
    };

    struct ClosureType : Type {
        ClosureType(vec<ptr<Type>> args, ptr<Type> result);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void emit(Printer* out) const override {
            result->emit(out);
            out->write("(*)(");
            for (size_t i = 0; i < args.size(); i++) {
                if (i) {
                    out->write(",");
                }
                args[i]->emit(out);
            }
        }

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

        virtual void emit(Printer* out) const override {
            name->emit(out);
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

        // TODO: lol
        virtual void emit(Printer* out) const override {
            names[0]->emit(out);
        }
    //private:
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
            out->section(str("- unary ") + get(), [&] {
                expr->visit(out);
            });
        }

        virtual void emit(Printer* out) const override {
            out->write("(");
            switch (op) {
                case NOT: out->write("!"); break;
                case FLIP: out->write("~"); break;
                case POS: out->write("+"); break;
                case NEG: out->write("-"); break;
                case DEREF: out->write("*"); break;
                case REF: out->write("&"); break;
                default: throw std::runtime_error("invalid unary op");
            }
            expr->emit(out);
            out->write(")");
        }

    private:
        const char* get() const {
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
            out->section(str("- binary `") + get() + "`", [&] {
                lhs->visit(out);
                rhs->visit(out);
            });
        }

        virtual void emit(Printer* out) const override {
            out->write("(");
            lhs->emit(out);
            switch (op) {
            case ADD: out->write("+"); break;
            case SUB: out->write("-"); break;
            case MUL: out->write("*"); break;
            case MOD: out->write("%"); break;
            case DIV: out->write("/"); break;
            case BITAND: out->write("&"); break;
            case BITOR: out->write("|"); break;
            case XOR: out->write("^"); break;
            case AND: out->write("&&"); break;
            case OR: out->write("||"); break;
            case SHL: out->write("<<"); break;
            case SHR: out->write(">>"); break;
            case LT: out->write("<"); break;
            case LTE: out->write("<="); break;
            case GT: out->write(">"); break;
            case GTE: out->write(">="); break;
            case EQ: out->write("=="); break;
            case NEQ: out->write("!="); break;
            default: throw std::runtime_error("invalid binary op");
            }
            rhs->emit(out);
            out->write(")");
        }

    private:
        const char* get() const {
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

        virtual void emit(Printer* out) const override {
            if (!yes) {
                throw std::runtime_error("elvis operator unimplemented");
            }

            cond->emit(out);
            out->write("?");
            yes->emit(out);
            out->write(":");
            no->emit(out);
        }
    private:
        ptr<Expr> cond;
        ptr<Expr> yes;
        ptr<Expr> no;
    };

    inline void replaceAll(str &s, const str &search, const str &replace ) {
        for (size_t pos = 0; ; pos += replace.length()) {
            pos = s.find(search, pos);
            if (pos == str::npos) 
                break;

            s.erase(pos, search.length());
            s.insert(pos, replace);
        }
    }

    struct StringExpr : Expr {
        StringExpr(const str* string);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void emit(Printer* out) const override {
            auto temp = *string;
            replaceAll(temp, "\n", "\\n");
            out->write("\"");
            out->write(temp);
            out->write("\"");
        }

    private:
        const str* string;
    };

    struct IntExpr : Expr {
        IntExpr(const Number& number);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void visit(Printer* out) const override {
            out->write("- int `" + std::to_string(number.number) + "`");
        }

        virtual void emit(Printer* out) const override {
            out->write(std::to_string(number.number));
            if (number.suffix) {
                out->write(*number.suffix);
            }
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

        virtual void emit(Printer* out) const override {
            out->write(val ? "1" : "0");
        }

    private:
        bool val;
    };

    struct CharExpr : Expr {
        CharExpr(c32 letter);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void emit(Printer* out) const override {
            out->write(std::to_string(letter));
        }

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

        // TODO: oh god
        virtual void emit(Printer* out) const override {
            name->names[0]->emit(out);
        }

    private:
        ptr<QualifiedType> name;
    };
    
    struct CoerceExpr : Expr {
        CoerceExpr(ptr<Type> type, ptr<Expr> expr);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void emit(Printer* out) const override {
            out->write("((");
            type->emit(out);
            out->write(")");
            expr->emit(out);
            out->write(")");
        }

    private:
        ptr<Type> type;
        ptr<Expr> expr;
    };

    struct SubscriptExpr : Expr {
        SubscriptExpr(ptr<Expr> expr, ptr<Expr> index);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void emit(Printer* out) const override {
            out->write("(");
            expr->emit(out);
            out->write("[");
            index->emit(out);
            out->write("])");
        }

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

        virtual void emit(Printer* out) const override {
            out->write("(");
            body->emit(out);
            out->write(indirect ? "->" : ".");
            field->emit(out);
            out->write(")");
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

        virtual void emit(Printer* out) const override {
            expr->emit(out);
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

        virtual void emit(Printer* out) const override {
            out->write("(");
            func->emit(out);
            out->write("(");
            for (size_t i = 0; i < args.size(); i++) {
                if (i) {
                    out->write(", ");
                }
                args[i]->emit(out);
            }
            out->write("))");
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

        virtual void emit(Printer* out) const override {
            out->write("typedef ");
            type->emit(out);
            name->emit(out);
            out->write(";");
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

        virtual void emit(Printer* out) const override {
            type->emit(out);
            out->write(" ");
            name->emit(out);
            out->write(";\n");
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

        virtual void emit(Printer* out) const override {
            out->write("struct ");
            name->emit(out);
            out->write("{\n");

            for (auto field : fields)
                field->emit(out);

            out->write("}\n");
        }

    private:
        ptr<Ident> name;
        vec<ptr<Field>> fields;
    };

    struct Union : Decl {
        Union(ptr<Ident> name, vec<ptr<Field>> fields);

        virtual bool equals(const ptr<Node> other) const override;

        virtual void emit(Printer* out) const override {
            out->write("union ");
            name->emit(out);
            out->write("{\n");

            for (auto field : fields)
                field->emit(out);

            out->write("}\n");
        }

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

        virtual void emit(Printer* out) const override {
            type->emit(out);
            out->write(" ");
            name->emit(out);
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

        virtual void emit(Printer* out) const override {
            result->emit(out);
            out->write(" ");
            name->emit(out);
            out->write("(");
            for (size_t i = 0; i < params.size(); i++) {
                if (i) {
                    out->write(", ");
                }
                params[i]->emit(out);
            }
            out->write(")\n");

            if (!body) {
                out->write(";");
                return;
            }

            out->write("{");
            if (auto expr = SELF<Expr>(body); expr) {
                out->write("return ");
                expr->emit(out);
                out->write(";");
            } else {
                body->emit(out);
            }
            out->write("}\n");
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

        virtual void emit(Printer* out) const override {
            out->write("{");
            for (auto stmt : stmts) {
                stmt->emit(out);
                out->write(";\n");
            }
            out->write("}");
        }

    private:
        vec<ptr<Stmt>> stmts;
    };

    struct If : Node {
        If(ptr<Expr> cond, ptr<Stmt> body);

        virtual bool equals(const ptr<Node> other) const override;
    private:
        ptr<Expr> cond;
        ptr<Stmt> body;
    };

    struct Branch : Stmt {
        Branch(vec<ptr<If>> branches);

        virtual bool equals(const ptr<Node> other) const override;
    private:
        vec<ptr<If>> branches;
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

        virtual void emit(Printer* out) const override {
            out->write("#include <");
            path[0]->emit(out);
            out->write(".h>");
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

        virtual void emit(Printer* out) const override {
            out->write("#ifndef UNIT_H\n#define UNIT_H\n");
            for (auto include : imports) {
                include->emit(out);
                out->write("\n");
            }

            for (auto decl : decls)
                decl->emit(out);

            out->write("#endif\n");
        }

    private:
        vec<ptr<Import>> imports;
        vec<ptr<Decl>> decls;
    };
}
