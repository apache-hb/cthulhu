#include "test.hpp"

int main() {
    /*
    auto* lex = cthulhu::string(u8R"(
        using std::mem(stack);
        using std::atomic(...);

        # hello
        using atomic_unit = atomic!<uint>;

        let counter = atomic_uint(0);

        let str = String("hello world");
        let len = str.length();

        let slice: span!<char> = str.slice(0, 5);

        struct Name {
            var yes: int;
        }

        union Name {
            
        }

        def fib(n: int): int = (n == 0 || n == 1) ? n : fib(n - 1) + fib(n - 2);
    )");
    auto parse = cthulhu::Parser(lex);

    cthulhu::Printer printer;

    auto* unit = parse.parseUnit();
    unit->visit(&printer);

    puts(printer.buffer.c_str());*/
}
