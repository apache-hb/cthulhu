#pragma once

#include "expr.h"

#include <map>
#include <vector>
#include <string>

namespace ctu
{
    struct Type
    {
        
    };

    struct Struct : Type
    {
        std::map<std::string, Type> fields;
    };

    struct Tuple : Type
    {
        std::vector<Type> fields;
    };

    struct Union : Type
    {
        std::vector<Type> fields;
    };

    struct Variant : Type
    {
        std::map<std::string, Type> fields;
    };

    /*
    struct Enum : Type
    {
        std::map<std::string, Expr> fields;
    };

    struct Array : Type
    {
        Type of;
        Expr size;
    };
    */

    struct Ptr : Type
    {
        Type to;
    };

    struct Name : Type
    {
        std::string name;
    };
}