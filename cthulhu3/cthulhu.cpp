#include "cthulhu.h"

#include <ctype.h>
#include <fmt/core.h>

namespace {
    bool isident1(char c) {
        return isalpha(c) || c == '_';
    }

    bool isident2(char c) {
        return isalnum(c) || c == '_';
    }

    bool isbase16(char c) {
        return isxdigit(c);
    }

    bool isbase2(char c) {
        return c == '0' || c == '1';
    }
    
    bool isbase10(char c) {
        return isdigit(c);
    }
}

namespace cthulhu {

text pool::intern(const std::string& string) {
    return &*data.insert(string).first;
}

// basic stream impls

file_stream::file_stream(const char* path) {
#ifdef _WIN32
    fopen_s(&file, path, "r");
#else
    file = fopen(path, "r");
#endif
}

file_stream::file_stream(FILE* file) 
    : file(file) 
{ }

file_stream::~file_stream() { }

char file_stream::next() {
    int c = fgetc(file);
    return c == -1 ? 0 : (char)c;
}

text_stream::text_stream(std::string_view string) 
    : string(string)
    , offset(0) 
{ }

text_stream::~text_stream() { }

char text_stream::next() {
    return offset >= string.length() ? 0 : string[offset++];
}


bool token::valid() const {
    return type < token::MONOSTATE && type != token::END;
}

bool token::error() const {
    return type >= token::MONOSTATE;
}

lexer::lexer(stream* source, std::shared_ptr<pool> idents)
    : source(source)
    , ahead(source->next())
    , idents(idents)
{ 
    keys = {
        { intern("record"), key::RECORD },
        { intern("using"), key::USING }
    };
}

token lexer::read() {
    char c = skip();
    token tok;

    if (isident1(c)) {
        tok = ident(c);
    } else if (c == 'R') {
        tok = rstring();
    } else if (isdigit(c)) {
        tok = digit(c);
    } else if (c == '"') {
        tok = string();
    } else if (c == '\'') {
        tok = letters();
    } else {
        tok = symbol(c);
    }

    return tok;
}

token lexer::ident(char c) {
    auto id = intern(collect(c, isident2));
    auto iter = keys.find(id);

    if (iter != keys.end()) {
        return make(token::KEY, { .word = iter->second });
    } else {
        return make(token::IDENT, { .id = id });
    }
}

token lexer::rstring() {
    return make(token::STRING);
}

token lexer::string() {
    auto kind = token::STRING;
    auto str = consume('"', &kind);

    return make(kind, { .str = str });
}

token lexer::letters() {
    auto kind = token::CHAR;
    auto str = consume('\'', &kind);

    return make(kind, { .letters = str });
}

token lexer::digit(char c) {
    std::string str;
    int base;

    if (c == '0' && eat('x')) {
        base = 16;
        str = collect(0, isbase16);
    } else if (c == '0' && eat('b')) {
        base = 2;
        str = collect(0, isbase2);
    } else if (c == '0' && isdigit(peek())) {
        return make(token::LEADING_ZERO);
    } else {
        base = 10;
        str = collect(c, isbase10);
    }

    auto suffix = isident1(peek()) ? intern(collect(c, isident2)) : nullptr;

    auto num = std::strtoull(str.c_str(), nullptr, base);

    if (errno == ERANGE) {
        return make(token::INT_OVERFLOW);
    }

    return make(token::INT, { .digit = { num, suffix } });
}

#define WORD(key) make(token::KEY, { .word = (key) })

token lexer::symbol(char c) {
    switch (c) {
    case '(': return WORD(key::LPAREN);
    case ')': return WORD(key::RPAREN);
    case '[': return WORD(key::LSQUARE);
    case ']': return WORD(key::RSQUARE);
    case '{': return WORD(key::LBRACE);
    case '}': return WORD(key::RBRACE);
    case ';': return WORD(key::SEMI);
    case ',': return WORD(key::COMMA);
    case '.': {
        if (eat('.')) {
            return WORD(eat('.') ? key::DOT3 : key::DOT2);
        } else {
            return WORD(key::DOT);
        }
    }
    case '+': return WORD(eat('=') ? key::ADDEQ : key::ADD);
    case '-': return WORD(eat('=') ? key::SUBEQ : key::SUB);
    case '/': return WORD(eat('=') ? key::DIVEQ : key::DIV);
    case '*': return WORD(eat('=') ? key::MULEQ : key::MUL);
    case '%': return WORD(eat('=') ? key::MODEQ : key::MOD);
    case '<': {
        if (eat('<')) {
            return WORD(eat('=') ? key::SHLEQ : key::SHL);
        } else {
            return WORD(eat('=') ? key::LTE : key::LT);
        }
    }
    case '>': {
        if (depth > 0) {
            depth--;
            return WORD(key::END);
        } else if (eat('>')) {
            return WORD(eat('=') ? key::SHREQ : key::SHR);
        } else {
            return WORD(eat('=') ? key::GTE : key::GT);
        }
    }
    case '?': return WORD(key::QUESTION);
    case '|': {
        if (eat('|')) {
            return WORD(key::OR);
        } else {
            return WORD(eat('=') ? key::BITOREQ : key::BITOR);
        }
    }
    case '&': {
        if (eat('&')) {
            return WORD(key::AND);
        } else {
            return WORD(eat('=') ? key::BITANDEQ : key::BITAND);
        }
    }
    case '^': return WORD(eat('=') ? key::BITXOREQ : key::BITXOR);
    case '=': return WORD(eat('=') ? key::EQ : key::ASSIGN);
    case '!': {
        if (eat('<')) {
            depth++;
            return WORD(key::BEGIN);
        } else {
            return WORD(eat('=') ? key::NEQ : key::NOT);
        }
    }
    case '~': return WORD(key::BITNOT);
    case '@': return WORD(key::AT);
    case ':': return WORD(eat(':') ? key::COLON2 : key::COLON);
    case 0: return make(token::END);
    default: return make(token::UNRECOGNIZED_CHAR);
    }
}

text lexer::consume(char delim, token::kind* kind) {
    std::string str;
    auto type = token::MONOSTATE;

    while (type == token::MONOSTATE) {
        char c = next();

        if (c == delim) {
            break;
        } else if (c == 0) {
            type = token::STRING_EOF;
            break;
        } else if (c == '\n') {
            type = token::STRING_LINE;
            break;
        } else if (c == '\\') {
            // string escape
            switch (peek()) {
            case '\\': str += '\\'; break;
            default: type = token::INVALID_ESCAPE;
            }
        } else {
            str += c;
        }
    }

    if (type != token::MONOSTATE) {
        *kind = type;
    }

    return intern(str);
}

token lexer::make(token::kind kind, token::content data) const {
    return { { start, offset - start }, kind, data };
}

char lexer::next() {
    char c = ahead;
    
    if (c) {
        ahead = source->next();
        text += c;
        offset += 1;
    }

    return c;
}

char lexer::peek() {
    return ahead;
}

char lexer::skip() {
    char c = next();

    while (true) {
        if (isspace(c)) {
            c = next();
        } else if (c == '#') {
            while (c != '\n')
                c = next();
        } else {
            break;
        }
    }

    start = offset - 1;

    return c;
}

bool lexer::eat(char c) {
    if (peek() == c) {
        next();
        return true;
    }
    return false;
}

text lexer::intern(const std::string& string) {
    return idents->intern(string);
}

std::string lexer::collect(char c, bool(*filter)(char)) {
    std::string str;

    if (c) { str += c; }

    while (peek() && filter(peek())) {
        str += next();
    }

    return str;
}

parser::parser(lexer* source)
    : source(source)
{ }

parser::~parser() {
    for (auto* node : nodes) {
        delete node;
    }
}

token parser::expect(token::kind type) {
    auto it = peek();

    if (it.type == type) {
        return next();
    }

    throw new error(error::EXPECT, { .expect = { token(type), it } });
}

token parser::expect(key key) {
    auto it = expect(token::KEY);
    if (it.data.word == key) {
        return it;
    }
    
    throw new error(error::EXPECT, { .expect = { token(token::KEY, { .word = key }), it } });
}

token parser::eat(key key) {
    auto it = peek();
    
    if (it.type == token::KEY && it.data.word == key) {
        return next();
    }

    return {};
}

// token stream managment

token parser::next() {
    auto it = peek();
    ahead = {};
    return it;
}

token parser::peek() {
    if (!ahead.valid()) {
        ahead = source->read();
    }

    return ahead;
}

// debugging/source reporting functions

std::string lexer::slice(const range& range) const {
    return text.substr(range.offset, range.length);
}


const char* token::repr() const {
    switch (type) {
    case token::IDENT: return "ident";
    case token::KEY: return "key";
    case token::STRING: return "string";
    case token::CHAR: return "char";
    case token::INT: return "int";
    case token::END: return "end";
    case token::MONOSTATE: return "monostate";
    case token::STRING_EOF: return "eof in text";
    case token::STRING_LINE: return "newline in text";
    case token::INVALID_ESCAPE: return "invalid escape sequence in text";
    case token::LEADING_ZERO: return "leading zero in int";
    case token::INT_OVERFLOW: return "int overflow";
    case token::UNRECOGNIZED_CHAR: return "invalid char";
    default: return "unknown";
    }
}

}
