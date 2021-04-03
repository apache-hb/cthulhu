#include <cthulhu.h>
#include <fstream>
#include <iostream>
#include <fmt/core.h>

using Paths = std::vector<fs::path>;

struct DefaultHandles : cthulhu::Handles {
    virtual ~DefaultHandles() { }

    DefaultHandles(Paths dirs) : dirs(dirs) { }

    Paths dirs;

    virtual std::optional<cthulhu::File> open(const fs::path& root, const fs::path& path) override {
        auto read = [](const fs::path& path) {
            auto search = fs::absolute(path);
            std::optional<cthulhu::File> file;
            
            if (std::ifstream in(search); !in.fail()) {
                file = { search, std::string(std::istreambuf_iterator<char>{in}, {}) };
            }
            
            return file;
        };

        // first search all include directories
        for (const auto& dir : dirs) {
            if (auto result = read(dir/path); result) {
                return result;
            }
        }

        // then try and read relative to the current file
        if (auto result = read(root/path); result) {
            return result;
        }

        // then try the absolute path last
        return read(path);
    }
};

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << ": no source files provided" << std::endl;
        return 1;
    }

    std::vector<fs::path> includes = { 
        fs::current_path(), // include the cwd
        fs::current_path()/"lib" // include the standard library
    };

    DefaultHandles handles(includes);

    try {
        cthulhu::init(&handles);

        auto ctx = cthulhu::compile(argv[1]);

    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
