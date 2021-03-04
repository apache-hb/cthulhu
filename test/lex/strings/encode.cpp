#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"(
        "hello \x55 world"
        "hello \d55 world"
    )");
    auto lexer = TestLexer(&stream);

    //lexer.expectString("hello \x55 world");
    //lexer.expectString("hello \55 world");

    //lexer.expect(Token::END);
}
