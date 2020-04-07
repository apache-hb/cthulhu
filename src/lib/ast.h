#pragma once

namespace ctu::ast
{
    struct node
    {
        
    };

    struct ident : node
    {
        char* name;
    };

    struct expr : node
    {

    };

    struct assign : node
    {
        ident* target;
        expr* value;
    };

    struct type
    {

    };

    struct typedecl : node
    {
        ident* name;
        type* val;
    };
}