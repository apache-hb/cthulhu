#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"(name "name" name2)");
    auto lexer = TestLexer(&stream);

    auto ident1 = lexer.expectIdent("name");
    auto ident2 = lexer.expectString("name");
    auto ident3 = lexer.expectIdent("name2");
    
    lexer.expect(Token::END);

    ASSERT(ident1 != ident3);
    ASSERT(ident1 != ident2);
}
