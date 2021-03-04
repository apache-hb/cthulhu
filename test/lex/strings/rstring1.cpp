#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = FileStream("data/plain/rstring1.txt");
    auto lexer = TestLexer(&stream);

    lexer.expectString("hello world");
    lexer.expect(Token::END);
}
