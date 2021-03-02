#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream("name name2 name");
    auto lexer = TestLexer(&stream);

    auto ident1 = lexer.expectIdent("name");
    auto ident2 = lexer.expectIdent("name2");
    auto ident3 = lexer.expectIdent("name");
    
    lexer.expect(Token::END);

    ASSERT(ident1 == ident3);
    ASSERT(ident2 != ident3);
    ASSERT(!(ident1 != ident3));
}
