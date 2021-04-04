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

    struct Handles {
        virtual ~Handles() { }

        // open a file and read in its source text
        // return std::nullopt if file doesn't exist
        // @param path: the path of the file to try and open
        virtual std::optional<std::string> open(const fs::path& path) = 0;
    };

    // init the global compiler state
    void init(Handles* handles);

    struct Include {
        // the name the context was imported with
        std::vector<std::string> name;

        // all symbols exposed from this include
        std::optional<std::vector<std::string>> items;

        // the context itself
        std::shared_ptr<Context> context;
    };

    struct Context : std::enable_shared_from_this<Context> {
        static std::shared_ptr<Context> open(const fs::path& path, const fs::path& cwd = fs::path());

        // create a new compilation unit
        Context(fs::path name, std::string source);

    private:
        // process just the include decls
        void includes();
        void include(const fs::path& path, const std::optional<std::vector<std::string>>& items);

        // the name of this compilation unit
        fs::path name;

        // the source text of this compilation unit
        std::string text;

        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;

        // all submodules this module directly depends on
        std::vector<Include> submodules;
    };
}
