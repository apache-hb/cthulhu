#include "test.hpp"

int main() {
    /*
    auto* lex = cthulhu::string(u8R"(
        # hello world
        # here are some comments
        # using a::b::c(a, b, c);

        struct something {
            def new(i: int): type {
                if (i > 100) {

                } else if (i == 200) {

                } else if (i % 10 == 2) {

                } else {
                    
                }
                
                return hello();
            }
        }
    )");
    auto parse = cthulhu::Parser(lex);

    cthulhu::Printer printer;

    auto* unit = parse.parseUnit();
    unit->visit(&printer);

    puts(printer.buffer.c_str());*/
}
