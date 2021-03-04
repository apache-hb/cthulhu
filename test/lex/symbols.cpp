#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("+ += !<>>>");
    auto lexer = TestLexer(&stream);

    lexer.expectKey(Key::ADD);
    lexer.expectKey(Key::ADDEQ);
    lexer.expectKey(Key::BEGIN);
    lexer.expectKey(Key::END);
    lexer.expectKey(Key::SHR);
    lexer.expect(Token::END);
}
