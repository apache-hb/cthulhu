#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("name");
    auto lexer = TestLexer(&stream);

    lexer.expectIdent("name");
    lexer.expect(Token::END);
}
