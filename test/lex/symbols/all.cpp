#include "tstream.hpp"
#include "tlexer.hpp"

#define OP(id, str) str " "

int main() {
    auto stream = StringStream(
#include <keys.inc>
    );
    auto lexer = TestLexer(&stream);

#define OP(id, _) lexer.expectKey(Key::id);

#include <keys.inc>

    lexer.expect(Token::END);
}
