#pragma once

#include <vector>
#include <string>
#include <variant>

namespace ct::ast
{
    using Path = std::vector<std::string>;

    struct Import
    {
        Path path;
        std::vector<std::string> deps;
    };

    struct Struct
    {

    };

    struct Var
    {

    };

    struct Func
    {

    };

    using Body = std::variant<Struct, Var, Func>;

    struct Program
    {
        std::vector<Import> imports;
        std::vector<Body> body;
    };
}