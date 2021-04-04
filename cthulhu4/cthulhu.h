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

    struct Symbol : std::enable_shared_from_this<Symbol> {
        enum Type {
            BUILTIN, // builtin types
            DEFER // order independent lookup
        } type;

        std::string name;

        Symbol(Type type, std::string name)
            : type(type)
            , name(name)
        { }
    };

    struct Include {
        // the name the context was imported with
        std::vector<std::string> name;

        // all symbols exposed from this include
        std::vector<std::string> items;

        // the context itself
        std::shared_ptr<Context> context;

        std::shared_ptr<Symbol> lookup(const std::string& name);
    };

    struct Context : std::enable_shared_from_this<Context> {
        static std::shared_ptr<Context> open(const fs::path& path, const fs::path& cwd = fs::path());

        // create a new compilation unit
        Context(fs::path name, std::string source);

    private:
        // process just the include decls
        void includes();
        void include(const fs::path& path, const std::vector<std::string>& items);

        // resolve all types
        void resolve();

        // register a type
        std::shared_ptr<Symbol> add(std::shared_ptr<Symbol> symbol);

        // lookup a type
        std::shared_ptr<Symbol> lookup(const std::string& path);

        // the name of this compilation unit
        fs::path name;

        // the source text of this compilation unit
        std::string text;

        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;

        // all submodules this module directly depends on
        std::vector<Include> submodules;

        // all symbols declared in this context
        std::vector<std::shared_ptr<Symbol>> symbols;
    };
}
