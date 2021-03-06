#include "tstream.hpp"
#include "tlexer.hpp"

#define KEY(id, str) str " "

int main() {
    auto stream = StringStream(
#include <keys.inc>
    );
    auto lexer = TestLexer(&stream);

#define KEY(id, _) lexer.expectKey(Key::id);

#include <keys.inc>

    lexer.expect(Token::END);
}
