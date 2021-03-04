#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("!<!<> >>>> !<> ! <> !>> <> <<>> <>>");
    auto lexer = TestLexer(&stream);

    lexer.expectKey(Key::BEGIN);
    lexer.expectKey(Key::BEGIN);
    lexer.expectKey(Key::END);
    lexer.expectKey(Key::END);
    lexer.expectKey(Key::SHR);
    lexer.expectKey(Key::GT);

    lexer.expectKey(Key::BEGIN);
    lexer.expectKey(Key::END);

    lexer.expectKey(Key::NOT);

    lexer.expectKey(Key::LT);
    lexer.expectKey(Key::GT);

    lexer.expectKey(Key::NOT);
    lexer.expectKey(Key::SHR);

    lexer.expectKey(Key::LT);
    lexer.expectKey(Key::GT);

    lexer.expectKey(Key::SHL);
    lexer.expectKey(Key::SHR);

    lexer.expectKey(Key::LT);
    lexer.expectKey(Key::SHR);

    lexer.expect(Token::END);
}
