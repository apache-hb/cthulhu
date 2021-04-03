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
    struct File {
        fs::path absolute;
        std::string text;
    };

    struct Handles {
        virtual ~Handles() { }

        // open a file and read in its source text
        // return std::nullopt if file doesnt exist
        // @param root: the path of the current module trying to import another module
        // @param path: the path of the file to try and open relative to root
        virtual std::optional<File> open(const fs::path& root, const fs::path& path) = 0;
    };

    // init the global compiler state
    void init(Handles* handles);

    struct Context : std::enable_shared_from_this<Context> {
        // create a new compilation unit
        // @param file: the file to compile
        // @param parent: the compilation unit that is importing this one
        Context(fs::path path, std::string text, std::shared_ptr<Context> parent);

        // perform compilation on this unit
        void process();

        // the folder this unit is in
        fs::path where() const;

    private:
        // the name of this compilation unit
        fs::path path;

        // the source text
        std::string text;

        // all modules that include this module
        std::shared_ptr<Context> parent;

        std::shared_ptr<Context> glob(const fs::path& name);

        // the ast from the source code
        std::shared_ptr<peg::Ast> tree;


        // handle an include decl
        void include(std::shared_ptr<peg::Ast> ast);

        // search and include a submodule
        std::shared_ptr<Context> search(std::shared_ptr<peg::Ast> ast);
    };

    // compile a compilation unit and all its submodules
    // @param file: the file to compile
    // @param parent: the module that is including this module
    std::shared_ptr<Context> compile(fs::path file, std::shared_ptr<Context> parent = nullptr);
}
