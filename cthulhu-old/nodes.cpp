#include "nodes.hpp"

#include "lexer.hpp"

namespace cthulhu {
    utf8::string Token::repr() const {
        utf8::string out = "Token(from=";
        out += range->lexer->name;
        out += ",at=(";
        out += std::to_string(range->line);
        out += ":";
        out += std::to_string(range->column);
        out +="))";;
        return out;
    }

#define ANSI_COLOUR_RED     "\x1b[31m"
#define ANSI_COLOUR_GREEN   "\x1b[32m"
#define ANSI_COLOUR_YELLOW  "\x1b[33m"
#define ANSI_COLOUR_BLUE    "\x1b[34m"
#define ANSI_COLOUR_MAGENTA "\x1b[35m"
#define ANSI_COLOUR_CYAN    "\x1b[36m"
#define ANSI_COLOUR_RESET   "\x1b[0m"

    utf8::string Token::underline(utf8::string error, utf8::string note) const {
        utf8::string out;
        
        utf8::string line = std::to_string(range->line);
        utf8::string pad = utf8::string(line.length() + 2, ' ');
        utf8::string text = range->lexer->line(range->offset);

        if (!error.empty()) {
            out += ANSI_COLOUR_MAGENTA "error: " ANSI_COLOUR_RESET;
            out += error;
            out += "\n";
        }

        out += " -> ";
        out += range->lexer->name;
        out += ":[";
        out += line;
        out += ":";
        out += std::to_string(range->column);
        out += "..";
        out += std::to_string(range->column + range->length);
        out += "]\n";

        out += pad;
        out += ANSI_COLOUR_CYAN "|\n ";
        out += line;
        out += " | " ANSI_COLOUR_RESET;
        out += text;
        out += "\n" ANSI_COLOUR_CYAN;
        out += pad;
        out += "| " ANSI_COLOUR_RED;
        out += utf8::string(range->column - 1, ' ');
        out += utf8::string(range->length, '^');
        
        if (!note.empty()) {
            out += " ";
            out += note;
        }

        out += "\n" ANSI_COLOUR_RESET;

        return out;
    }
}
