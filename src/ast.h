#pragma once

#include <vector>
#include <string>

namespace ct::ast
{
    using Path = std::vector<std::string>;

    struct Import
    {
        Path path;
        std::vector<std::string> deps;
    };

    struct Program
    {
        std::vector<Import> imports;
    };
}