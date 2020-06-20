#pragma once

#include <variant>

namespace ct::ast {
    struct Node {
        enum { 

        } type;

        Token parent;
    };

    struct Type : Node {
        enum { 
            BUILTIN,
            STRUCT,
            NAME
        } ttype;
    };

    struct Builtin : Type {
        enum { } builtin;
    };

    struct Struct : Type {
        struct Field {
            std::string name;
            std::shared_ptr<Type> type;
        };

        std::vector<Field> fields;
    };

    struct Name : Type {
        std::vector<std::string> path;
    };

    struct Import : Node {
        std::vector<std::string> path;
        std::vector<std::string> items;
    };

    struct Stmt : Node {};

    struct Expr : Stmt {};

    struct Int : Expr {
        int val;
    };

    struct Return : Stmt {
        std::unique_ptr<Expr> val;
    };

    struct Function : Node {
        struct Arg {
            std::string name;
            std::shared_ptr<Type> type;
        };

        std::vector<Arg> args;
        std::shared_ptr<Type> ret;

        std::unique_ptr<Stmt> body;
    };

    struct Unit : Node {
        std::vector<Import> imports;
        std::map<std::string, std::shared_ptr<Type>> types;
        std::map<std::string, std::shared_ptr<Function>> funcs;
    };
}
