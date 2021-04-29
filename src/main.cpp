#include <vector>
#include <string>
#include <iostream>

#include "cthulhu.h"

struct str : cthulhu::stream::handle {
    virtual ~str() = default;

    str(std::string src)
        : idx(0)
        , src(src)
    { }

    virtual char next() override {
        return idx > src.length() ? 0 : src[idx++];
    }

    size_t idx;
    std::string src;
};

int compile(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cerr << args[0] << ": requires at least 1 parameter" << std::endl;
        return 1;
    }
    
    str source = { args[1] };

    cthulhu::lexer lexer = { &source };

    while (true) {
        auto tok = lexer.read();
        std::cout << tok.debug() << std::endl;
        if (tok.type == cthulhu::token::END) {
            break;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++)
        args.push_back(argv[i]);

    return compile(args);
}
