#pragma once

#include <vector>
#include <string>
#include <variant>

namespace ct::ast
{
    struct Node
    {

    };

    struct Id : Node
    {
        std::string content;
    };
}