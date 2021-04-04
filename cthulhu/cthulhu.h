#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <filesystem>
#include <optional>
#include <unordered_set>
#include <peglib.h>

namespace fs = std::filesystem;

namespace cthulhu {
    struct Context;

    // init the global compiler state
    void init();

    enum struct SymbolType {
        DEFER, // order independent lookup
        DEFINE, // defined symbol
    };

    struct Symbol {
        SymbolType type;
        std::string name;
    };

    struct Context {
        // create a new compilation unit
        Context(std::string source);

        // resolve names
        void resolve();

    private:
        // define a symbol
        void define(const std::string& name);

        // defer a symbol
        void defer(const std::string& name);

        // check if a symbol is already defined
        bool lookup(const std::string& name);

        // the source text of this compilation unit
        std::string text;

        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;

        // all symbols declared in this context
        // start off with just builtin types
        std::vector<Symbol> symbols = {
            { SymbolType::DEFINE, "int" },
            { SymbolType::DEFINE, "void" }
        };
    };
}
