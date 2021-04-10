#include <cthulhu.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <fmt/core.h>

using namespace cthulhu;

struct X86: Context {
    X86() : Context() {

        // these are all the builtin types
        builtins = {
            new ast::VoidType(),
            new ast::BoolType(),

            new ast::ScalarType("char", 1, true),
            new ast::ScalarType("short", 2, true),
            new ast::ScalarType("int", 4, true),
            new ast::ScalarType("long", 8, true),

            new ast::ScalarType("uchar", 1, false),
            new ast::ScalarType("ushort", 2, false),
            new ast::ScalarType("uint", 4, false),
            new ast::ScalarType("ulong", 8, false),
        };
    }
};

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << ": no source files provided" << std::endl;
        return 1;
    }

    auto path = argv[1];

    if (std::ifstream in(path); !in.fail()) {
        auto text = std::string(std::istreambuf_iterator<char>{in}, {});
        
        try {
            init();

            X86 ctx;

            auto source = Builder(text);

            source.build(&ctx);
        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "failed to open: " << path << std::endl;
        return 1;
    }
}
