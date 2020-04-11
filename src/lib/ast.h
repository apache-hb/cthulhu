#pragma once

#include <vector>
#include <string>

namespace ctu::ast
{
    struct Node
    {
        
    };

    struct Ident : Node
    {
        Ident(std::string n)
            : name(n)
        {}

        std::string name;
    };

    struct Name : Node
    {
        std::vector<Ident*> path;
    };


    struct Expr : Node
    {

    };

    struct Attribute
    {
        Name* path;
    };

#pragma region Types

    struct Type
    {
        std::vector<Attribute*> attribs;
    };

    struct Field 
    {
        char* name;
        Type* type;
    };

    struct Struct :  Type
    {
        int count;
        Field* fields;
    };

    struct Tuple : Type
    {
        int count;
        Type* fields;
    };

    struct Entry 
    {
        char* name;
        Expr* value;
    };

    struct Enum : Type
    {
        int count;
        Entry* fields;
    };

    struct Variant : Type
    {
        int count;
        Field* fields;
    };

#pragma endregion Types

#pragma region Constructs

    struct Import
    {
        Name* path;
        Ident* alias;
    };

    struct TypeDef : Node
    {
        Ident* name;
        Type* type;
    };

#pragma endregion Constructs

    struct TopLevel
    {
        int count;
        Node* fields;
    };

    struct Program
    {
        Program(std::vector<Import*> i)
            : imports(i)
        {}
        
        std::vector<Import*> imports;
    };
}