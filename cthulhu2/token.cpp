#include "token.h"
#include "lexer.h"
#include "util.h"

#include <algorithm>
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

#define ANSI_COLOUR_BOLD    "\x1b[1m"

std::string Token::repr() {
    switch (type) {
    case Token::IDENT: 
        return fmt::format("{}@ident({})", (void*)data.ident, *data.ident);
    case Token::KEY: 
        return fmt::format("key({})", ::repr(data.key));
    case Token::STRING: 
        return fmt::format("{}@string(`{}`)", (void*)data.string, *data.string);
    case Token::CHAR: return "char";
    case Token::INT: 
        if (data.number.suffix) {
            return fmt::format("{}@int({}, {})", 
                (void*)data.number.suffix, 
                data.number.number, 
                *data.number.suffix
            );
        } else {
            return fmt::format("int({})", data.number.number);
        }
    case Token::END: return "eof";
    
    case Token::ERROR_STRING_EOF: return "unterminated string";
    case Token::ERROR_STRING_LINE: return "newline in string";
    case Token::ERROR_INVALID_ESCAPE: return "invalid escape sequence";
    case Token::ERROR_LEADING_ZERO: return "leading zero in integer";
    case Token::ERROR_INT_OVERFLOW: return "integer literal was too large";
    default: return "invalid";
    }
}

std::string Token::text() {
    return range.lexer->text.substr(range.offset, range.length);
}

std::string Token::pretty(bool underline, bool colour, const std::string& message) {
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

    std::string src;
    size_t length = range.length;

    len = std::to_string(where.line + lines.size()).length() + 2;
    pad = "\n" + std::string(len, ' ') + "|";

    switch (lines.size()) {
    case 1:
        src = line + " > " + join(lines, "\n" + std::string(len, ' ') + "| ");
        break;
    case 2: case 3: {
        size_t i = where.line;
        for (const auto& each : lines) {
            if (i != where.line) {
                src += "\n ";
            }
            src += std::to_string(i++) + " > " + each;
        }
        length = std::max_element(lines.begin(), lines.end(), [](auto& lhs, auto& rhs) { return lhs.length() > rhs.length(); })->length();
        break;
    }
    default: {
        length = std::max_element(lines.begin(), lines.end(), [](auto& lhs, auto& rhs) { return lhs.length() < rhs.length(); })->length();
        auto& first = lines[0];
        auto& last = lines[lines.size() - 1];
        auto end = std::to_string(where.line + lines.size());
        src += std::string(end.length() - line.length(), ' ') + line + " > " + first + "\n";
        src += std::string(len - 1, ' ') + "...\n ";
        src += end + " > " + last;
        break;
    }
    }

    if (colour) {
        src = ANSI_COLOUR_BOLD ANSI_COLOUR_WHITE + src + ANSI_COLOUR_RESET;
    }

    auto result = header + pad + "\n " + src + pad;

    if (underline && length) {
        result += std::string(where.column + 1, ' ') + ANSI_COLOUR_CYAN + std::string(length, '^') + ANSI_COLOUR_RESET + " " + message + pad;
    }

    return result;
}

bool Token::error() const {
    return type >= Token::ERROR;
}
