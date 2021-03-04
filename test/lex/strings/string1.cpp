#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"(
        "hello world"
    )");
    auto lexer = TestLexer(&stream);

    lexer.expectString("hello world");

    lexer.expect(Token::END);
}
