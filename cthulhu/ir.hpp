#pragma once

#include "ast.hpp"
#include <map>
#include <string>
#include <vector>
#include <stddef.h>

//
// intermediate representation of language
// these are created by validating an AST then creating an ir
//

namespace cthulhu::ir {
    using namespace std;

    // this is a full unit with everything required
    // to make a final executable, including all included modules
    // and all dependencies
    // is this a slow way of managing stuff? yeah probably
    // but this compiler is for self hosting, not for being good
    struct Function {

    };

    struct Type {
        utf8::string name;
    };
    
    struct Unit {
        utf8::string name;
        vector<Type*> types;
    };

    Unit validate(cthulhu::Unit* root, Unit(*include)(vector<utf8::string>));
}
