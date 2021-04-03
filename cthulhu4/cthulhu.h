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
    struct Handles {
        virtual ~Handles() { }

        // open a file and read in its source text
        // return std::nullopt if file doesnt exist
        // @param path: the path of the file to try and open
        virtual std::optional<std::string> open(const fs::path& path) = 0;
    };

    // init the global compiler state
    void init(Handles* handles);

    struct Context : std::enable_shared_from_this<Context> {
        static std::shared_ptr<Context> open(const fs::path& path, const fs::path& cwd = fs::path());

        // create a new compilation unit
        Context(fs::path path, std::string source);

    private:
        void process();
        void include(std::shared_ptr<peg::Ast> ast);

        // the name of this compilation unit
        fs::path path;

        // the source text of this compilation unit
        std::string text;

        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;

    };
}
