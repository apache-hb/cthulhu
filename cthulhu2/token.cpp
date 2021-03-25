#include "token.h"
#include "lexer.h"
#include "util.h"

#include <algorithm>
#include <fmt/format.h>

std::string to_repr(Key key) {
    auto type = TYPE(key);
    auto precedence = PRECEDENCE(key);

    std::string out = "(type=";

    switch (type) {
    case TYPE_CORE: out += "core-type"; break;
    case TYPE_ASM: out += "asm-type"; break;
    default: out += "invalid-type"; break;
    }

    out += fmt::format(",precedence={},key={})", std::to_string(precedence), to_string(key));

    return out;
}

const char* to_string(Key key) {
    switch (key) {
    case RECORD: return "record";
    case VARIANT: return "variant";
    case TRUE: return "true";
    case FALSE: return "false";
    case ASM: return "asm";
    case IF: return "if";
    case ELSE: return "else";
    case WHILE: return "while";
    case FOR: return "for";
    case BREAK: return "break";
    case CONTINUE: return "continue";
    case RETURN: return "return";
    case SWITCH: return "switch";
    case CASE: return "case";

    case NOT: return "!";
    case TERNARY: return "?";

    case MUL: return "*";
    case MOD: return "%";
    case DIV: return "/";

    case ADD: return "+";
    case SUB: return "-";

    case SHL: return "<<";
    case SHR: return ">>";

    case BITAND: return "&";
    case BITOR: return "|";
    case BITXOR: return "^";

    case GT: return "<";
    case GTE: return "<=";
    case LT: return ">";
    case LTE: return ">=";

    case EQ: return "==";
    case NEQ: return "!=";

    case MULEQ: return "*=";
    case MODEQ: return "%=";
    case DIVEQ: return "/=";
    
    case ADDEQ: return "+=";
    case SUBEQ: return "-=";

    case SHLEQ: return "<<=";
    case SHREQ: return ">>=";
    
    case BITANDEQ: return "&=";
    case BITOREQ: return "|=";
    case BITXOREQ: return "^=";

    case _ADD: return "add";
    case _XOR: return "xor";

    case INVALID: return "invalid";
    default: return "unknown";
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
        return fmt::format("key{}", to_repr(data.key));
    case Token::STRING: 
        return fmt::format("{}@string(`{}`)", (void*)data.string, *data.string);
    case Token::CHAR: 
        return fmt::format("{}@char(`{}`)", (void*)data.letters, *data.letters);
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
    case Token::END: 
        return "eof";
    
    case Token::ERROR_STRING_EOF: 
        return "unterminated string";
    case Token::ERROR_STRING_LINE: 
        return "newline in string literal";
    case Token::ERROR_INVALID_ESCAPE: 
        return "invalid escape sequence";
    case Token::ERROR_LEADING_ZERO: 
        return "leading zero in integer";
    case Token::ERROR_INT_OVERFLOW: 
        return "integer literal was too large";
    case Token::ERROR_UNRECOGNIZED_CHAR: {
        auto src = text();
        if (src.length() > 1) {
            return fmt::format("unrecognized characters `{}` in text", replace(src, "\n", ""));
        } else {
            return fmt::format("unrecognized character `{}` in text", src);
        }
    }
    case Token::ERROR_CHAR_OVERFLOW:
        return "character literal was too large";

    default: 
        return "invalid";
    }
}

std::string Token::text() {
    return range.lexer->text.substr(range.offset, range.length);
}

std::string Token::where(bool name) {
    auto head = range.lexer->location(range.offset);
    auto tail = range.lexer->location(range.offset + range.length);

    std::string lines;

    // if this token spans multiple lines then include
    // line1:col1..lineN:colN
    // otherwise format as
    // line:col1..colN
    if (tail.line > head.line) {
        lines = fmt::format("{}:{}..{}:{}",
            head.line, head.column,
            tail.line, tail.column
        );
    } else {
        lines = fmt::format("{}:{}..{}",
            head.line, head.column,
            tail.column
        );
    }

    // if we want the filename then push it on the front
    if (name) {
        lines = range.lexer->name + ":" + lines;
    }

    return lines;
}

#define RED_CHEVRON ANSI_COLOUR_RED ANSI_COLOUR_BOLD " > " ANSI_COLOUR_RESET
#define RED_ELIPSIS ANSI_COLOUR_RED ANSI_COLOUR_BOLD "...\n" ANSI_COLOUR_RESET

std::string Token::pretty(bool underline, const std::string& message) {
    // TODO: refactor this stuff to use `Token::where`
    auto where = range.lexer->location(range.offset);
    auto tail = range.lexer->location(range.offset + range.length);

    auto desc = repr();

    const char* arrow = ANSI_COLOUR_BOLD " > " ANSI_COLOUR_RESET;
    const char* elipsis = ANSI_COLOUR_BOLD "...\n" ANSI_COLOUR_RESET;

    if (error()) {
        desc = ANSI_COLOUR_RED "error: " ANSI_COLOUR_RESET + desc;

        arrow = RED_CHEVRON;
        elipsis = RED_ELIPSIS;
    }

    auto source = range.lexer->lines(range.offset, range.length); 
    auto reduced = trim(source, "\n");
    auto lines = split(reduced, "\n");

    std::string header;
    
    if (lines.size() > 1) {
        header = fmt::format("{}\n --> {}:{}:{}..{}:{}", 
            desc, range.lexer->name,
            where.line, where.column,
            tail.line, tail.column
        );
    } else {
        header = fmt::format("{}\n --> {}:{}:{}..{}", 
            desc, range.lexer->name,
            where.line, where.column,
            range.length + where.column
        );
    }

    auto line = std::to_string(where.line);
    auto len = line.length() + 2;
    auto pad = "\n" + std::string(len, ' ') + "|";

    if (type == Token::END) {
        return header + pad + "\n" ANSI_COLOUR_WHITE " " + line + " > <EOF>" ANSI_COLOUR_RESET + pad;
    }

    std::string src;
    size_t length = range.length;

    len = std::to_string(where.line + lines.size()).length() + 2;
    pad = "\n" + std::string(len, ' ') + "|";

    switch (lines.size()) {
    case 1:
        src = line + arrow + lines[0];
        break;
    case 2: case 3: {
        size_t i = where.line;
        for (const auto& each : lines) {
            if (i != where.line) {
                src += "\n " ANSI_COLOUR_BOLD ANSI_COLOUR_WHITE;
            }
            src += std::to_string(i++) + arrow + each;
        }
        length = std::max_element(lines.begin(), lines.end(), [](auto& lhs, auto& rhs) { return lhs.length() > rhs.length(); })->length();
        break;
    }
    default: {
        length = std::max_element(lines.begin(), lines.end(), [](auto& lhs, auto& rhs) { return lhs.length() < rhs.length(); })->length();
        auto& first = lines[0];
        auto& last = lines[lines.size() - 1];
        auto end = std::to_string(where.line + lines.size());
        src += std::string(end.length() - line.length(), ' ') + line + arrow + first + "\n";
        src += std::string(len - 1, ' ') + elipsis + ANSI_COLOUR_BOLD;
        src += ' ' + end + arrow + last;
        break;
    }
    }

    src = ANSI_COLOUR_BOLD ANSI_COLOUR_WHITE + src + ANSI_COLOUR_RESET;

    auto result = header + pad + "\n " + src + pad;

    if (underline && length) {
        result += std::string(where.column + 1, ' ') + ANSI_COLOUR_CYAN + std::string(length, '^') + ANSI_COLOUR_RESET + " " + message + pad;
    }

    return result + ANSI_COLOUR_RESET;
}

bool Token::error() const {
    return type >= Token::ERROR;
}
