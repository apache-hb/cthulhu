#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = FileStream("data/rstring3.txt");
    auto lexer = TestLexer(&stream);

    lexer.expectString("\nhello world\n");
    lexer.expect(Token::END);
}
