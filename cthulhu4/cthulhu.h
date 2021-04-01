#pragma once

#include <string>

namespace cthulhu {
    struct Record {

    };

    struct Unit {

    };

    // parse source text into a possibly invalid ast
    Unit parse(const std::string& source);

    // name resolution
    inline void resolve_names(Unit* unit) { (void)unit; }

    // type resolution
    inline void resolve_types(Unit* unit) { (void)unit; }

    // do some optimizing of the AST
    // just constant folding for now
    inline void optimize_ast(Unit* unit) { (void)unit; }

    // emit code 
    inline void emit(Unit* unit) { (void)unit; }
}
