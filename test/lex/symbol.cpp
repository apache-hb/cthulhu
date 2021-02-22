#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8" + += . .. ... [] !< >> !< !< >> ");

    lex->expect<cthulhu::Key>(cthulhu::Key::ADD);
    lex->expect<cthulhu::Key>(cthulhu::Key::ADDEQ);
    lex->expect<cthulhu::Key>(cthulhu::Key::DOT);
    lex->expect<cthulhu::Key>(cthulhu::Key::DOT2);
    lex->expect<cthulhu::Key>(cthulhu::Key::DOT3);
    lex->expect<cthulhu::Key>(cthulhu::Key::LSQUARE);
    lex->expect<cthulhu::Key>(cthulhu::Key::RSQUARE);
    lex->expect<cthulhu::Key>(cthulhu::Key::BEGIN);
    lex->expect<cthulhu::Key>(cthulhu::Key::END);
    lex->expect<cthulhu::Key>(cthulhu::Key::GT);
    lex->expect<cthulhu::Key>(cthulhu::Key::BEGIN);
    lex->expect<cthulhu::Key>(cthulhu::Key::BEGIN);
    lex->expect<cthulhu::Key>(cthulhu::Key::END);
    lex->expect<cthulhu::Key>(cthulhu::Key::END);
    lex->expect<cthulhu::End>();
}
