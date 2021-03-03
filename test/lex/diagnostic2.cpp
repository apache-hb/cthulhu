#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"("hello\nworld")");
    auto lexer = TestLexer(&stream);

    lexer.read();

    auto msg = lexer.diagnostic();

    ASSERT(!msg.has_value());

    lexer.expect(Token::END);
}
