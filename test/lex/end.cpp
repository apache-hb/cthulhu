#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("");
    auto lexer = TestLexer(&stream);

    lexer.expect(Token::END);
    lexer.expect(Token::END);
}
