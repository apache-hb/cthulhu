#include "token.h"
#include "lexer.h"
#include "util.h"

#include <fmt/format.h>

namespace {
    const char* repr(Key key) {
        switch (key) {
#define KEY(id, str) case Key::id: return #id ": " str;
#define ASM(id, str) case Key::id: return #id ": " str;
#define OP(id, str) case Key::id: return #id ": " str;
#include "keys.inc"
        default: return "invalid";
        }
    }
}

#define ANSI_COLOUR_RED     "\x1b[31m"
#define ANSI_COLOUR_GREEN   "\x1b[32m"
#define ANSI_COLOUR_YELLOW  "\x1b[33m"
#define ANSI_COLOUR_BLUE    "\x1b[34m"
#define ANSI_COLOUR_MAGENTA "\x1b[35m"
#define ANSI_COLOUR_CYAN    "\x1b[36m"
#define ANSI_COLOUR_WHITE   "\x1b[97m"
#define ANSI_COLOUR_RESET   "\x1b[0m"

std::string Token::repr() {
    switch (type) {
    case Token::IDENT: return fmt::format("{}@ident({})", (void*)data.ident, *data.ident);
    case Token::KEY: return fmt::format("key({})", ::repr(data.key));
    case Token::STRING: return fmt::format("{}@string(`{}`)", (void*)data.string, *data.string);
    case Token::CHAR: return "char";
    case Token::INT: return "int";
    case Token::END: return "eof";
    
    case Token::ERROR_STRING_EOF: return "unterminated string";
    case Token::ERROR_STRING_LINE: return "newline in string";
    default: return "invalid";
    }
}

std::string Token::text() {
    return range.lexer->text.substr(range.offset, range.length);
}

std::string Token::pretty(bool underline, bool colour) {
    auto where = range.lexer->location(range.offset);

    auto desc = repr();

    if (error()) {
        if (colour) {
            desc = ANSI_COLOUR_RED "error: " ANSI_COLOUR_RESET + desc;
        } else {
            desc = "error: " + desc;
        }
    }

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
        return header + pad + "\n" ANSI_COLOUR_WHITE " " + line + " > <EOF>" ANSI_COLOUR_RESET + pad;
    }

    auto source = range.lexer->lines(range.offset, range.length); 

    auto reduced = trim(source, "\n");

    auto lines = split(reduced, "\n");

    auto text = join(lines, "\n" + std::string(len, ' ') + "| ");

    auto src = line + " > " + text;
    if (colour) {
        src = ANSI_COLOUR_WHITE + src + ANSI_COLOUR_RESET;
    }

    auto result = header + pad + "\n " + src + pad;

    if (underline) {
        result += std::string(where.column + 1, ' ') + ANSI_COLOUR_CYAN + std::string(range.length, '^') + ANSI_COLOUR_RESET + pad;
    }

    return result;
}

bool Token::error() const {
    return type >= Token::ERROR;
}
