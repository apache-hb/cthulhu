
#if 0

namespace cthulhu {
    using namespace std;

    struct Expr;
    struct Type;
    struct Qual;
    struct Decl;

    struct Node {
        virtual ~Node() { }

        virtual void visit(Printer* out) const = 0;
    };

    struct Stmt : Node {

    };

    struct Compound : Stmt {
        vector<Stmt*> items;

        Compound(vector<Stmt*> stmts)
            : items(stmts)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- compound");
            out->enter([&] {
                for (Stmt* stmt : items) {
                    stmt->visit(out);
                }
            });
        }
    };

    struct Expr : Stmt {

    };

    struct IntConst : Expr {
        Int* num;

        IntConst(Int* i)
            : num(i)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- int");
            out->enter([&] {
                out->write("- value `" + std::to_string(num->get()) + "`");
                if (!num->suf().empty()) {
                    out->write("- suffix");
                    out->enter([&] {
                        out->write(num->suf());
                    });
                }
            });
        }
    };

    struct StrConst : Expr {
        String* val;

        StrConst(String* val)
            : val(val)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- string \"" + val->get() + "\"");
        }
    };

    struct BoolConst : Expr {
        Key* val;

        BoolConst(Key* key)
            : val(key)
        { }

        virtual void visit(Printer* out) const override {
            out->write(val->key == Key::TRUE ? "- bool `true`" : "- bool `false`");
        }
    };

    struct CharConst : Expr {
        Char* val;

        CharConst(Char* val)
            : val(val)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- char `" + utf8::string(val->get()) + "`");
        }
    };

    struct Ternary : Expr {
        Expr* cond;
        Expr* lhs;
        Expr* rhs;

        Ternary(Expr* cond, Expr* lhs, Expr* rhs)
            : cond(cond)
            , lhs(lhs)
            , rhs(rhs)
        { }

        virtual void visit(Printer* out) const override  {
            out->write("- ternary");
            out->enter([&] {
                out->write("- condition");
                out->enter([&] {
                    cond->visit(out);
                });
                
                if (lhs) {
                    out->write("- true");
                    out->enter([&] {
                        lhs->visit(out);
                    });
                }

                out->write("- false");
                out->enter([&] {
                    rhs->visit(out);
                });
            });
        }
    };

    struct Subscript : Expr {
        Expr* expr;
        Expr* index;

        Subscript(Expr* expr, Expr* index)
            : expr(expr)
            , index(index)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- subscript");
            out->enter([&] {
                out->write("- expr");
                out->enter([&] {
                    expr->visit(out);
                });

                out->write("- index");
                out->enter([&] {
                    index->visit(out);
                });
            });
        }
    };

    struct Unary : Expr {
        enum Op {
            INVALID,
            NOT, // !
            FLIP, // ~
            POS, // + 
            NEG, // -
            DEREF, // *
            REF // &
        };

        Op op;
        Expr* expr;

        Unary(Op op, Expr* expr)
            : op(op)
            , expr(expr)
        { }

        utf8::string str() const { 
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

        virtual void visit(Printer* out) const override {
            out->write("- unary `" + str() + "`");
            out->enter([&] {
                out->enter([&] {
                    expr->visit(out);
                });
            });
        }
    };

    struct Binary : Expr {
        enum Op {
            INVALID,
            ADDEQ, // +=
            SUBEQ, // -=
            DIVEQ, // /=
            MULEQ, // *=
            MODEQ, // %=
            BITANDEQ, // &=
            BITOREQ, // |=
            BITXOREQ, // ^=
            SHLEQ, // <<=
            SHREQ, // >>=
            ADD, // +
            SUB, // -
            MUL, // *
            DIV, // /
            MOD, // %
            BITAND, // &
            BITOR, // |
            BITXOR, // ^
            AND, // &&
            OR, // ||
            SHL, // <<
            SHR, // >>
            LT, // <
            LTE, // <=
            GT, // >
            GTE, // >=
            EQ, // ==
            NEQ, // !=
            MACRO // !
        };

        Op op;
        Expr* lhs;
        Expr* rhs;

        Binary(Op op, Expr* lhs, Expr* rhs)
            : op(op)
            , lhs(lhs)
            , rhs(rhs)
        { }

        utf8::string str() const { 
            switch (op) {
            case ADDEQ: return "ADDEQ";
            case SUBEQ: return "SUBEQ";
            case DIVEQ: return "DIVEQ";
            case MULEQ: return "MULEQ";
            case MODEQ: return "MODEQ";
            case BITANDEQ: return "BITANDEQ";
            case BITOREQ: return "BITOREQ";
            case BITXOREQ: return "BITXOREQ";
            case SHLEQ: return "SHLEQ";
            case SHREQ: return "SHREQ";
            case ADD: return "ADD";
            case SUB: return "SUB";
            case MUL: return "MUL";
            case DIV: return "DIV";
            case MOD: return "MOD";
            case BITAND: return "BITAND";
            case BITOR: return "BITOR";
            case BITXOR: return "XOR";
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
            case MACRO: return "MACRO";
            default: return "INVALID";
            }
        }

        virtual void visit(Printer* out) const override {
            out->write("- binary");
            out->enter([&] {
                out->write("- op `" + str() + "`");
                out->write("- lhs");
                out->enter([&] {
                    lhs->visit(out);
                });
                out->write("- rhs");
                out->enter([&] {
                    rhs->visit(out);
                });
            });
        }
    };

    struct Type : Node {

    };

    struct Name : Type {
        Name(Ident* tok)
            : name(tok)
        { }

        Name(Ident* tok, vector<Type*> params)
            : params(params)
            , name(tok)
        { }

        vector<Type*> params;

        Ident* name;

        virtual void visit(Printer* out) const override {
            out->write("- name `" + name->get() + "`");
            out->enter([&] {
                if (!params.empty()) {
                    out->write("- params");
                    out->enter([&] {
                        for (Type* type : params) {
                            type->visit(out);
                        }
                    });
                }
            });
        }
    };

    struct Qual : Type {
        Qual(vector<Name*> names)
            : names(names)
        { }

        vector<Name*> names;
        
        virtual void visit(Printer* out) const override {
            out->write("- qual");
            out->enter([&] {
                for (Name* name : names) {
                    name->visit(out);
                }
            });
        }
    };

    struct Pointer : Type {
        Pointer(Type* type)
            : type(type)
        { }

        Type* type;

        virtual void visit(Printer* out) const override {
            out->write("- pointer");
            out->enter([&] {
                type->visit(out);
            });
        }
    };

    struct Array : Type {
        Array(Type* type, Expr* size)
            : type(type)
            , size(size)
        { }

        Type* type;
        Expr* size;

        virtual void visit(Printer* out) const override {
            out->write("- array");
            out->enter([&] {
                out->write("- type");
                out->enter([&] {
                    type->visit(out);
                });

                if (size) {
                    out->write("- size");
                    out->enter([&] {
                        size->visit(out);
                    });
                }
            });
        }
    };

    struct Closure : Type {
        Type* type;
        vector<Type*> params;

        Closure(Type* type, vector<Type*> params)
            : type(type)
            , params(params)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- closure");
            out->enter([&] {
                out->write("- result");
                out->enter([&] {
                    type->visit(out);
                });

                out->write("- args");
                out->enter([&] {
                    for (Type* type : params) {
                        type->visit(out);
                    }
                });
            });
        }
    };

    struct Coerce : Expr {
        Type* type;
        Expr* expr;

        Coerce(Type* type, Expr* expr)
            : type(type)
            , expr(expr)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- coerce");
            out->enter([&] {
                out->write("- type");
                out->enter([&] {
                    type->visit(out);
                });
                out->write("- expr");
                out->enter([&] {
                    expr->visit(out);
                });
            });
        }
    };

    struct NameExpr : Expr {
        Qual* name;

        NameExpr(Qual* qual)
            : name(qual)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- name");
            out->enter([&] {
                name->visit(out);
            });
        }
    };

    struct FunctionParam : Node {
        Ident* key;
        Expr* expr;

        FunctionParam(Ident* key, Expr* expr)
            : key(key)
            , expr(expr)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- param");
            out->enter([&] {
                if (key) {
                    out->write("- key");
                    out->enter([&] {
                        out->write(key->get());
                    });
                }

                out->write("- expr");
                out->enter([&] {
                    expr->visit(out);
                });
            });
        }
    };

    struct Call : Expr {
        Expr* expr;
        vector<FunctionParam*> args;

        Call(Expr* expr, vector<FunctionParam*> args)
            : expr(expr)
            , args(args)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- call");
            out->enter([&] {
                out->write("- function");
                out->enter([&] {
                    expr->visit(out);
                });

                out->write("- args");
                out->enter([&] {
                    for (FunctionParam* arg : args) {
                        arg->visit(out);
                    }
                });
            });
        }
    };

    struct Dot : Expr {
        Expr* lhs;
        Ident* name;
        bool ptr;

        Dot(Expr* lhs, Ident* name, bool ptr)
            : lhs(lhs)
            , name(name)
            , ptr(ptr)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- dot");
            out->enter([&] {
                out->write("- expr");
                out->enter([&] {
                    lhs->visit(out);
                });
                out->write("- field");
                out->enter([&] {
                    out->write(name->get());
                });
                out->write("- indirect");
                out->enter([&] {
                    out->write(ptr ? "true" : "false");
                });
            });
        }
    };

    struct Return : Stmt {
        Expr* expr;

        Return(Expr* expr)
            : expr(expr)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- return");
            if (expr) {
                out->enter([&] {
                    expr->visit(out);
                });
            }
        }
    };
    
    struct While : Stmt {
        Expr* cond;
        Stmt* body;

        While(Expr* cond, Stmt* body)
            : cond(cond)
            , body(body)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- while");
            out->enter([&] {
                out->write("- cond");
                out->enter([&] {
                    cond->visit(out);
                });
                out->write("- body");
                out->enter([&] {
                    body->visit(out);
                });
            });
        }
    };

    // for (var i .. range(0, 10))
    struct ForRange : Stmt {
        Decl* name;
        Expr* iter;
        Stmt* body;
    };

    // for (var i = 0; i < 10; i += 1)
    struct ForLoop : Stmt {
        Decl* init;
        Expr* check;
        Expr* next;
        Stmt* body;
    };

    struct If : Stmt {
        struct Branch {
            Expr* cond;
            Stmt* body;
        };

        vector<Branch> branches;

        If(vector<Branch> branches)
            : branches(branches)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- branch");
            out->enter([&] {
                for (Branch branch : branches) {
                    if (branch.cond) {
                        out->write("- if");
                        out->enter([&] {
                            branch.cond->visit(out);
                        });
                    } else {
                        out->write("- else");
                    }
                    out->enter([&] {
                        branch.body->visit(out);
                    });
                }
            });
        }
    };

    struct Switch : Stmt {
        struct Case {
            Expr* expr;
            Stmt* body;
        };

        vector<Case> cases;
    };

    struct Decl : Node {

    };

    struct Import : Decl {
        vector<Ident*> path;
        vector<Ident*> items;
        bool block;

        Import(vector<Ident*> path)
            : path(path)
            , items({})
            , block(true)
        { }

        Import(vector<Ident*> path, vector<Ident*> items)
            : path(path)
            , items(items)
            , block(false)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- import");
            out->enter([&] {
                out->write("- path");
                out->enter([&] {
                    for (Ident* ident : path) {
                        out->write(ident->get());
                    }
                });

                if (block) {
                    out->write("- block");
                } else {
                    out->write("- items");
                    out->enter([&] {
                        if (items.empty()) {
                            out->write("...");
                        } else {
                            for (Ident* ident : items) {
                                out->write(ident->get());
                            }
                        }
                    });
                }
            });
        }
    };

    struct Alias : Decl {
        Ident* name;
        Type* type;

        Alias(Ident* name, Type* type)
            : name(name)
            , type(type)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- alias");
            out->enter([&] {
                out->write("- name");
                out->enter([&] {
                    out->write(name->get());
                });
                out->write("- type");
                out->enter([&] {
                    type->visit(out);
                });
            });
        }
    };

    struct VarName : Decl {
        Ident* name;
        Type* type;

        VarName(Ident* name, Type* type)
            : name(name)
            , type(type)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- name");
            out->enter([&] {
                out->write(name->get());
            });

            if (type) {
                out->write("- type");
                out->enter([&] {
                    type->visit(out);
                });
            }
        }
    };

    struct Var : Decl {
        vector<VarName*> names;
        Expr* expr;
        bool mut;

        Var(vector<VarName*> names, Expr* expr, bool mut)
            : names(names)
            , expr(expr)
            , mut(mut)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- variable");
            out->enter([&] {
                out->write("- mutable");
                out->enter([&] {
                    out->write(mut ? "true" : "false");
                });
                
                out->write("- names");
                out->enter([&] {
                    for (VarName* name : names) {
                        name->visit(out);
                    }
                });

                if (expr) {
                    out->write("- init");
                    out->enter([&] {
                        expr->visit(out);
                    });
                }
            });
        }
    };

    struct Decorator : Decl {
        Qual* name;
        vector<FunctionParam*> params;

        Decorator(Qual* name, vector<FunctionParam*> params)
            : name(name)
            , params(params)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- decorator");
            out->enter([&] {
                out->write("- name");
                out->enter([&] {
                    name->visit(out);
                });

                out->enter([&] {
                    out->write("- params");
                    out->enter([&] {
                        for (FunctionParam* param : params) {
                            param->visit(out);
                        }
                    });
                });
            });
        }
    };

    template<typename T>
    struct Decorated : T {
        vector<Decorator*> decorators;

        // this is a stupid hack that doubles memory usage
        // but it works so whatever
        T* item;

        Decorated(vector<Decorator*> decorators, T* item)
            : T(*item)
            , decorators(decorators)
            , item(item)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- decorated");
            out->enter([&] {
                for (Decorator* decorator : decorators) {
                    decorator->visit(out);
                }

                out->write("- body");
                out->enter([&] {
                    item->visit(out);
                });
            });
        }
    };

    struct With : Stmt {
        Stmt* init;
        Stmt* body;

        With(Stmt* init, Stmt* body)
            : init(init)
            , body(body)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- with");
            out->enter([&] {
                out->write("- init");
                out->enter([&] {
                    init->visit(out);
                });

                out->write("- body");
                out->enter([&] {
                    body->visit(out);
                });
            });
        }
    };

    struct FunctionArg : Node {
        Ident* name;
        Type* type;
        Expr* init;

        FunctionArg(Ident* name, Type* type, Expr* init)
            : name(name)
            , type(type)
            , init(init)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- arg `" + name->get() + "`");
            out->enter([&] {
                out->write("- type");
                out->enter([&] {
                    type->visit(out);
                });
                if (init) {
                    out->write("- init");
                    out->enter([&] {
                        init->visit(out);
                    });
                }
            });
        }
    };

    struct Function : Decl {
        Qual* name;
        vector<FunctionArg*> args;
        Type* result;
        Node* body;

        Function(Qual* name, vector<FunctionArg*> args, Type* result, Node* body)
            : name(name)
            , args(args)
            , result(result)
            , body(body)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- function");
            out->enter([&] {
                out->write("- name");
                out->enter([&] {
                    name->visit(out);
                });
                out->write("- args");
                out->enter([&] {
                    for (auto* arg : args) {
                        arg->visit(out);
                    }
                });
                out->write("- result");
                out->enter([&] {
                    result->visit(out);
                });
                if (body) {
                    out->write("- body");
                    out->enter([&] {
                        body->visit(out);
                    });
                }
            });
        }
    };

    struct Struct : Decl {
        Ident* name;
        vector<Decl*> fields;

        Struct(Ident* name, vector<Decl*> fields)
            : name(name)
            , fields(fields)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- struct " + name->get());
            out->enter([&] {
                out->write("- fields");
                out->enter([&] {
                    for (Decl* decl : fields) {
                        decl->visit(out);
                    }
                });
            });
        }
    };

    struct Union : Decl {
        Ident* name;
        vector<Decl*> fields;

        Union(Ident* name, vector<Decl*> fields)
            : name(name)
            , fields(fields)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- union " + name->get());
            out->enter([&] {
                out->write("- fields");
                out->enter([&] {
                    for (Decl* decl : fields) {
                        decl->visit(out);
                    }
                });
            });
        }
    };

    struct EnumField : Decl {
        Ident* name;
        Expr* value;
    };

    struct Enum : Decl {
        vector<Decl*> fields;
    };

    struct TaggedEnumField : Decl {
        Ident* name;
        vector<Decl*> fields;
        Expr* value;
    };

    struct TaggedEnum : Decl {
        vector<Decl*> fields;
    };

    struct TemplateParam : Node {
        Ident* name;
        vector<Qual*> limits;

        TemplateParam(Ident* name, vector<Qual*> limits)
            : name(name)
            , limits(limits)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- template-param " + name->get());
        }
    };

    struct Template : Decl {
        Decl* of;
        vector<TemplateParam*> params;

        Template(Decl* of, vector<TemplateParam*> params)
            : of(of)
            , params(params)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- template");
            out->enter([&] {
                out->write("- params");
                out->enter([&] {
                    for (TemplateParam* param : params) {
                        param->visit(out);
                    }
                });

                out->write("- decl");
                out->enter([&] {
                    of->visit(out);
                });
            });
        }
    };

    struct Unit : Node {
        vector<Import*> includes;
        vector<Decl*> decls;

        Unit(vector<Import*> includes, vector<Decl*> decls)
            : includes(includes)
            , decls(decls)
        { }

        virtual void visit(Printer* out) const override {
            out->write("- unit");
            out->enter([&] {
                for (Import* include : includes) {
                    include->visit(out);
                }

                for (Decl* decl : decls) {
                    decl->visit(out);
                }
            });
        }
    };
}

#endif