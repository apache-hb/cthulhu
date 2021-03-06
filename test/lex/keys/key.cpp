#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("name using usingyes");
    auto lexer = TestLexer(&stream);

    lexer.expectIdent("name");
    lexer.expectKey(Key::USING);
    lexer.expectIdent("usingyes");
    lexer.expect(Token::END);
}
