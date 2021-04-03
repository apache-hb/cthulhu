#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <filesystem>
#include <peglib.h>

namespace cthulhu {
    struct Handles {
        virtual ~Handles() { }
        virtual std::string open(const std::filesystem::path& path) = 0;
    };

    // init the global compiler state
    void init(Handles* handles);

    struct Symbol {
        enum Type {
            SCALAR, /* builtin types like char, int, bool, str, void, etc */
            DEFER, /* order independent lookup */
            RECORD, UNION, VARIANT,
            POINTER, ARRAY,
        } type;

        Symbol(Type kind)
            : type(kind)
        { }
    };

    struct Context : std::enable_shared_from_this<Context> {
        Context(const std::filesystem::path& name);

        void compile();

    private:
        // name of the file being compiled
        std::filesystem::path name;
        // the source code of the file
        std::string source;
        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;

        // include
        void include(const std::shared_ptr<peg::Ast> ast);

        // open a context or grab it from cache
        std::shared_ptr<Context> open(const std::filesystem::path& path);

        // all included modules
        std::vector<std::shared_ptr<Context>> includes;
    };
}
