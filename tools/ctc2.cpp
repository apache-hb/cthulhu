#include <cthulhu.h>
#include <ast.h>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << ": no source files provided" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    if (std::ifstream in(path); !in.fail()) {
        auto text = std::string(std::istreambuf_iterator<char>{in}, {});
        
        try {
            ctu::init();

            ctu::Context ctx = ctu::parse(text, {
                new ctu::Builtin("int")
            });

            for (auto scope : ctx.scopes) {
                for (auto symbol : scope.symbols) {
                    std::cout << symbol->debug() << std::endl;
                }
            }
        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "failed to open: " << path << std::endl;
        return 1;
    }
}
