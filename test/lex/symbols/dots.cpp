#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(". .. ... . ...");
    auto lexer = TestLexer(&stream);

    lexer.expectKey(Key::DOT);
    lexer.expectKey(Key::DOT2);
    lexer.expectKey(Key::DOT3);
    lexer.expectKey(Key::DOT);
    lexer.expectKey(Key::DOT3);
    lexer.expect(Token::END);
}
