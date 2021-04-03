#include <cthulhu.h>
#include <fstream>
#include <iostream>
#include <fmt/core.h>

struct DefaultHandles : cthulhu::Handles {
    virtual ~DefaultHandles() { }

    virtual std::string open(const std::filesystem::path& path) override {
        if (std::ifstream in(path); !in.fail()) {
            return std::string(std::istreambuf_iterator<char>{in}, {});
        } 

        throw std::runtime_error(fmt::format("failed to open `{}`", path.string()));
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

        auto ctx = std::make_shared<cthulhu::Context>(argv[1]);
        ctx->compile();

    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
