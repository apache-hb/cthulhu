#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        def fib(n: int): int = (n == 0 || n == 1) ? n : fib(n-1) + fib(n-2);

        def main(argc: int, argv: **char): int {
            printf("fib(10) = %d\n", fib(10));
        }
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.parseUnit();

    parse.finish();
}
