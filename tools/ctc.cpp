//#include <cthulhu.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <fmt/core.h>

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << ": no source files provided" << std::endl;
        return 1;
    }

    /*
    auto path = argv[1];

    if (std::ifstream in(path); !in.fail()) {
        auto text = std::string(std::istreambuf_iterator<char>{in}, {});
        
        try {
            cthulhu::init();

            auto ctx = std::make_shared<cthulhu::Context>(text);

            ctx->parse();

            std::cout << "parsed file: " << path << std::endl;

            ctx->sema();

            std::cout << "validated program" << std::endl;

            ctx->emit();

        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "failed to open: " << path << std::endl;
        return 1;
    }*/
}
