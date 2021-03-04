#include "tstream.hpp"
#include "tlexer.hpp"

int main() {
    auto stream = StringStream(R"("name" "name" "name2")");
    auto lexer = TestLexer(&stream);

    auto str1 = lexer.expectString("name");
    auto str2 = lexer.expectString("name");
    auto str3 = lexer.expectString("name2");
    
    lexer.expect(Token::END);

    ASSERT(str1 == str2);
    ASSERT(str2 != str3);
    ASSERT(str1 != str3);
}
