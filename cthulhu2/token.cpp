#include "token.h"
#include "lexer.h"
#include "util.h"

#include <fmt/format.h>

std::string Token::repr() {
    switch (type) {
    case Token::IDENT: return fmt::format("{}@ident({})", (void*)data.ident, *data.ident);
    case Token::KEY: return "key";
    case Token::STRING: return fmt::format("{}@string(\"{}\"", (void*)data.string, *data.string);
    case Token::CHAR: return "char";
    case Token::INT: return "int";
    case Token::END: return "eof";
    default: return "invalid";
    }
}

std::string Token::text() {
    return range.lexer->text.substr(range.offset, range.length);
}

std::string Token::pretty(bool underline) {
    auto where = range.lexer->location(range.offset);

    auto desc = repr();

    auto header = fmt::format("{}:{}:{}\n --> {}", 
        range.lexer->name,
        where.line, 
        where.column,
        desc
    );

    auto line = std::to_string(where.line);
    auto len = line.length() + 2;
    auto pad = "\n" + std::string(len, ' ') + "|";

    if (type == Token::END) {
        return header + "\n " + line + " | <EOF>\n";
    }

    auto source = range.lexer->lines(range.offset, range.length); 

    auto reduced = trim(source, "\n");

    auto lines = split(reduced, "\n");

    auto text = join(lines, "\n" + std::string(len, ' ') + "| ");

    auto result = header + pad + "\n " + line + " | " + text + pad;

    if (underline) {
        result += std::string(where.column + 1, ' ') + std::string(range.length, '^') + pad;
    }

    return result;
}
