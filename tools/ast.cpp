#include <memory>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cthulhu.h>

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << " <file> " << std::endl;
        return 1;
    }

    std::ifstream in(argv[1]);
    auto text = std::string(std::istreambuf_iterator<char>{in}, {});

    try {
        auto unit = cthulhu::parse(text);

        resolve_names(&unit);
        resolve_types(&unit);
        optimize_ast(&unit);
        emit(&unit);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
