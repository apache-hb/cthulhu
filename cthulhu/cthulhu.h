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

    struct Context {
        // create a new compilation unit
        Context(std::string source);

    private:
        // the source text of this compilation unit
        std::string text;

        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;
    };
}
