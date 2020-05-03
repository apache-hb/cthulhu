#pragma once

#include <map>
#include <vector>
#include <string>
#include <optional>

struct Node {

};

struct Import : Node {
    std::vector<std::string> parts;
    std::optional<std::string> alias;
};

struct Stmt : Node {

};

struct Expr : Node {

};

struct Type : Stmt {

};

struct Func : Stmt {
    Type ret;
};

struct Name : Type {
    std::vector<std::string> name;
};

struct Builtin : Type { 
    enum {
        BOOL,
        U8,
        U16,
        U32,
        U64,
        I8,
        I16,
        I32,
        I64,
        CHAR
    } type;
};

struct Ptr : Type {
    Type to;
};

struct Struct : Type {
    std::map<std::string, Type> fields;
};

struct Union : Type {
    std::map<std::string, Type> fields;
};

struct Enum : Type {
    std::map<std::string, Expr> values;
};

struct Array : Type {
    Type type;
    Expr size;
};

struct Variant : Type {
    std::map<std::string, Type> fields;
};

struct Object : Type {
    std::map<std::string, Type> fields;
    std::map<std::string, Func> methods;
    std::vector<Func> inits;
};


struct Program : Node {
    std::vector<Import> deps;
    std::map<std::string, Stmt> body;
};
