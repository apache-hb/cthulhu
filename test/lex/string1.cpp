#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"(
        "hello world"
    )");
    auto lexer = TestLexer(&stream);

    

    lexer.expect(Token::END);
}
