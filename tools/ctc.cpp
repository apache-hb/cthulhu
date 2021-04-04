#include <cthulhu.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <fmt/core.h>

using paths = std::vector<fs::path>;

struct DefaultHandles : cthulhu::Handles {
    virtual ~DefaultHandles() { }

    virtual std::optional<std::string> open(const fs::path& path) override {
        std::optional<std::string> file;
        
        if (std::ifstream in(path); !in.fail()) {
            file = std::string(std::istreambuf_iterator<char>{in}, {});
        }

        return file;
    }
};

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << ": no source files provided" << std::endl;
        return 1;
    }

    DefaultHandles handles;

    try {
        cthulhu::init(&handles);

        auto ctx = cthulhu::Context::open(argv[1]);

    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    std::cout << "done" << std::endl;
}
