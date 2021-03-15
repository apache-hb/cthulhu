#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>

#include "test/tstream.hpp"

int main(int argc, char** argv) {
    if (argc < 1) {
        printf("ast <in> [out]\n");
        exit(1);
    }
    
    FileStream stream{argv[1]};
    Lexer lexer{&stream, argv[1]};
    Parser parser{&lexer};
    auto unit = parser.parseUnit();

    if (!unit) {
        printf("failed to parse file\n");
        exit(1);
    }

    ast::Printer out;
    out.feed = false;
    unit->emit(&out);

    if (argc > 2) {
        std::fstream f(argv[2]);
        f << out.buffer;
        f.close();
    } else {
        puts(out.buffer.c_str());
    }
}
