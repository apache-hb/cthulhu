#include "cthulhu.hpp"

#include <unordered_map>

namespace cthulhu {
    bool whitespace(char32_t c) {
        switch (c) {
        case 0x0009:
        case 0x000A:
        case 0x000B:
        case 0x000C:
        case 0x000D:
        case 0x001C:
        case 0x001D:
        case 0x001E:
        case 0x001F:
        case 0x0020:
        case 0x0085:
        case 0x00A0:
        case 0x1680:
        case 0x2000:
        case 0x2001:
        case 0x2002:
        case 0x2003:
        case 0x2004:
        case 0x2005:
        case 0x2006:
        case 0x2007:
        case 0x2008:
        case 0x2009:
        case 0x200A:
        case 0x2028:
        case 0x2029:
        case 0x202F:
        case 0x205F:
        case 0x3000:
            return true;

        default:
            return false;
        }
    }

    bool newline(char32_t c) {
        switch (c) {
        case 0x000A:
        case 0x000B:
        case 0x000C:
        case 0x000D:
        case 0x001C:
        case 0x001D:
        case 0x001E:
        case 0x0085:
        case 0x2028:
        case 0x2029:
            return true;

        default:
            return false;
        }
    }

    bool isident1(char32_t c) {
        return isalpha(c) || c == '_';
    }

    bool isident2(char32_t c) {
        return isalnum(c) || c == '_';
    }

    ///
    /// stream handling
    ///
    Stream::Stream(StreamHandle* handle)
        : handle(handle)
        , lookahead(handle->next())
    { }

    char32_t Stream::next() {
        char32_t temp = lookahead;
        lookahead = handle->next();
        return temp;
    }

    char32_t Stream::peek() {
        return lookahead;
    }



    ///
    /// lexing
    ///

    utf8::string Lexer::slice(Range* range) {
        return text.substr(range->offset, range->length);
    }

    utf8::string Lexer::line(size_t it) {
        size_t offset = it;
        while (true) {
            if (offset <= 0)
                break;

            if (newline(text[offset]))
                break;

            offset--;
        }

        size_t length = 0;
        while (true) {
            if (offset + length >= text.length())
                break;

            if (newline(text[offset + length]))
                break;

            length++;
        }

        return text.substr(offset, std::min(length, text.length()));
    }

    char32_t Lexer::skip() {
        char32_t c = next();

        while (whitespace(c)) {
            c = next();
        }

        return c;
    }

    char32_t Lexer::next() {
        char32_t c = stream.next();

        if (c == END) {
            return END;
        }

        if (newline(c)) {
            here.line++;
            here.column = 0;
        } else {
            here.column++;
        }

        here.offset++;

        text += c;

        return c;
    }

    char32_t Lexer::peek() {
        return stream.peek();
    }

    bool Lexer::eat(char32_t c) {
        if (peek() == c) {
            next();
            return true;
        }

        return false;
    }

    char32_t Lexer::escape() {
        char32_t c = next();
        if (c == '"') {
            return END;
        } else if (c == '\\') {
            return next();
        } else {
            return c;
        } 
    }

    utf8::string Lexer::collect(char32_t start, bool(*filter)(char32_t)) {
        utf8::string out = start == END ? u8"" : utf8::string(start);

        while (filter(peek())) {
            out += next();
        }

        return out;
    }

    using KeyMap = unordered_map<utf8::string, Key::Word>;

#define KEY(id, str) { u8 ## str, Key::id },

    const KeyMap keys = {
#include "keys.inc"
    };

    Token* Lexer::read() {
        char32_t c = skip();
        Location start = here;
        start.offset--;
        Token* token = nullptr;

        if (c == END) {
            token = new End();
        } else if (isident1(c)) {
            utf8::string str = collect(c, [](char32_t c) { return isident2(c); });

            if (auto iter = keys.find(str); iter == keys.end()) {
                token = new Ident(str);
            } else {
                token = new Key(iter->second);
            }

        } else if (isdigit(c)) {
            utf8::string str;
            uint64_t num;

            if (c == '0' && eat('x')) {
                str = collect(END, [](char32_t c) -> bool { return isxdigit(c); });
                num = strtoull(str.c_str(), nullptr, 16);
            } else if (c == '0' && eat('b')) {
                str = collect(END, [](char32_t c) { return c == '0' || c == '1'; });
                num = strtoull(str.c_str(), nullptr, 2);
            } else {
                str = collect(c, [](char32_t c) -> bool { return isdigit(c); });
                num = strtoull(str.c_str(), nullptr, 10);
            }

            if (isident1(peek())) {
                utf8::string suffix = collect(next(), [](char32_t c) { return isident2(c); });
                token = new Int(num, suffix);
            } else {
                token = new Int(num);
            }
        } else if (c == '"') {
            utf8::string str;
            while (true) {
                c = escape();
                if (c == END) {
                    break;
                } else {
                    str += c;
                }
            }
            token = new String(str);
        } else if (c == '\'') {
            c = escape();
            c = c == END ? '"' : c;
            
            if (next() != '\'') {
                throw new std::runtime_error("expected ' after char literal");
            }

            token = new Char(c);
        } else {
            Key::Word key = Key::INVALID;

            switch (c) {
            case '[':
                key = Key::LSQUARE;
                break;

            case ']':
                key = Key::RSQUARE;
                break;

            case '(':
                key = Key::LPAREN;
                break;

            case ')':
                key = Key::RPAREN;
                break;

            case '{':
                key = Key::LBRACE;
                break;

            case '}':
                key = Key::RBRACE;
                break;

            case '.':
                key = eat('.') ? (eat('.') ? Key::DOT3 : Key::DOT2) : Key::DOT;
                break;

            case ',':
                key = Key::COMMA;
                break;

            case '!':
                if (eat('<')) {
                    depth++;
                    key = Key::BEGIN;
                } else {
                    key = eat('=') ? Key::NEQ : Key::NOT;
                }
                break;
            
            case '<':
                if (eat('<')) {
                    key = eat('=') ? Key::SHLEQ : Key::SHL;
                } else {
                    key = eat('=') ? Key::LTE : Key::LT;
                }
                break;

            case '>':
                if (depth > 0) {
                    depth--;
                    key = Key::END;
                } else {
                    if (eat('>')) {
                        key = eat('=') ? Key::SHREQ : Key::SHR;
                    } else {
                        key = eat('=') ? Key::GTE : Key::GT;
                    }
                }
                break;

            case '+': 
                key = eat('=') ? Key::ADDEQ : Key::ADD;
                break;

            case '-':
                key = eat('>') ? Key::ARROW : eat('=') ? Key::SUBEQ : Key::SUB;
                break;

            case '*':
                key = eat('=') ? Key::MULEQ : Key::MUL;
                break;

            case '/':
                key = eat('=') ? Key::DIVEQ : Key::DIV;
                break;

            case '%':
                key = eat('=') ? Key::MODEQ : Key::MOD;
                break;

            case '|':
                key = eat('|') ? Key::OR : eat('=') ? Key::BITOREQ : Key::BITOR;
                break;

            case '&':
                key = eat('&') ? Key::AND : eat('=') ? Key::BITANDEQ : Key::BITAND;
                break;

            case '?':
                key = Key::QUESTION;
                break;

            case '^':
                key = eat('=') ? Key::XOREQ : Key::XOR;
                break;

            case '=':
                key = eat('=') ? Key::EQ : Key::ASSIGN;
                break;

            case '~':
                key = Key::FLIP;
                break;

            case ':':
                key = eat(':') ? Key::COLON2 : Key::COLON;
                break;

            case ';':
                key = Key::SEMI;
                break;

            default:
                break;
            }

            token = new Key(key);
        }

        token->range = start.to(here);
        return token;
    }


    ///
    /// token data
    ///

    Location::Location(Lexer* lexer, size_t offset, size_t line, size_t column) 
        : lexer(lexer) 
        , offset(offset) 
        , line(line) 
        , column(column) 
    { }

    Range* Location::to(Location other) {
        return new Range(lexer, offset, line, column, other.offset - offset);
    }

    Range::Range(Lexer* lexer, size_t offset, size_t line, size_t column, size_t length)
        : Location(lexer, offset, line, column)
        , length(length)
    { }

    ///
    /// parsing logic
    ///

    Unit* Parser::unit() {
        vector<Import*> includes;
        vector<Decl*> decls;

        while (true) {
            Decl* it = include();
            if (!it) {
                break;
            }

            if (dynamic_cast<Alias*>(it)) {
                decls.push_back(it);
                break;
            } else {
                includes.push_back(dynamic_cast<Import*>(it));
            }
        }

        while (true) {
            Decl* d = decl();
            if (!d) {
                break;
            }
            decls.push_back(d);
        }

        return new Unit(includes, decls);
    }

    Type* Parser::type() {
        Type* out = nullptr;

        if (Key* mul = eat<Key>(Key::MUL); mul) {
            out = new Pointer(type());
        } else if (Key* lsquare = eat<Key>(Key::LSQUARE); lsquare) {
            out = array();
        } else {
            out = qual();
        }

        while (eat<Key>(Key::LPAREN)) {
            vector<Type*> args = gather<Type>(Key::COMMA, Key::RPAREN, [](Parser* self) {
                return self->type();
            });
            out = new Closure(out, args);
        }

        return out;
    }

    Array* Parser::array() {
        Array* out;
        Type* it = type();
        if (Key* colon = eat<Key>(Key::COLON); colon) {
            out = new Array(it, expr());
        } else {
            out = new Array(it, nullptr);
        }
        expect<Key>(Key::RSQUARE);
        return out;
    }

    enum Prec : int {
        P_NONE = 0,

        P_ASSIGN,
        P_MACRO,
        P_TERNARY,
        P_LOGIC,
        P_EQUAL,
        P_COMPARE,
        P_BITS,
        P_SHIFT,
        P_MATH,
        P_MUL
    };

    int prec(Key* word) {
        if (word == nullptr)
            return P_NONE;

        switch (word->key) {
        case Key::ADD: case Key::SUB:
            return P_MATH;
        case Key::MUL: case Key::DIV: case Key::MOD:
            return P_MUL;
        case Key::EQ: case Key::NEQ:
            return P_EQUAL;
        case Key::AND: case Key::OR:
            return P_LOGIC;
        case Key::GT: case Key::GTE: case Key::LT: case Key::LTE:
            return P_COMPARE;
        case Key::SHL: case Key::SHR:
            return P_SHIFT;
        case Key::BITAND: case Key::BITOR: case Key::XOR:
            return P_BITS;
        case Key::QUESTION:
            return P_TERNARY;
        case Key::NOT:
            return P_MACRO;
        default: 
            return P_NONE;
        }
    }

    Unary::Op unop(Key* key) {
        if (key == nullptr)
            return Unary::INVALID;

        switch (key->key) {
        case Key::NOT: return Unary::NOT;
        case Key::FLIP: return Unary::FLIP;
        case Key::ADD: return Unary::POS;
        case Key::SUB: return Unary::NEG;
        case Key::MUL: return Unary::DEREF;
        case Key::BITAND: return Unary::REF;
        default: return Unary::INVALID;
        }
    }

    Binary::Op binop(Key* key) {
        if (key == nullptr)
            return Binary::INVALID;

        switch (key->key) {
        case Key::ADD: return Binary::ADD;
        case Key::SUB: return Binary::SUB;
        case Key::MUL: return Binary::MUL;
        case Key::DIV: return Binary::DIV;
        case Key::MOD: return Binary::MOD;
        case Key::BITAND: return Binary::BITAND;
        case Key::BITOR: return Binary::BITOR;
        case Key::XOR: return Binary::BITXOR;
        case Key::AND: return Binary::AND;
        case Key::OR: return Binary::OR;
        case Key::SHL: return Binary::SHL;
        case Key::SHR: return Binary::SHR;
        case Key::LT: return Binary::LT;
        case Key::LTE: return Binary::LTE;
        case Key::GT: return Binary::GT;
        case Key::GTE: return Binary::GTE;
        case Key::EQ: return Binary::EQ;
        case Key::NEQ: return Binary::NEQ;
        case Key::NOT: return Binary::MACRO;
        default: return Binary::INVALID;
        }
    }

    int assoc(int prec) {
        return prec != P_ASSIGN;
    }

    Expr* Parser::expr() {
        return binary(P_ASSIGN);
    }

    Expr* Parser::binary(int mprec) {
        Expr* lhs = primary();

        while (true) {
            Token* tok = peek();
            Key* key = dynamic_cast<Key*>(tok);

            int nprec = prec(key);
            if (nprec == P_NONE || nprec < mprec)
                break;

            next();

            Expr* rhs;

            if (nprec == P_TERNARY) {
                if (eat<Key>(Key::COLON)) {
                    rhs = new Ternary(lhs, nullptr, expr());
                } else {
                    Expr* it = expr();
                    expect<Key>(Key::COLON);
                    lhs = new Ternary(lhs, it, expr());
                }
            } else {
                rhs = binary(nprec + assoc(nprec));
                lhs = new Binary(binop(key), lhs, rhs);
            }
        }

        return lhs;
    }

    FunctionParam* Parser::funcParam() {
        if (eat<Key>(Key::DOT)) {
            Ident* key = expect<Ident>();
            expect<Key>(Key::ASSIGN);
            return new FunctionParam(key, expr());
        } else {
            return new FunctionParam(nullptr, expr());
        }
    }

    Expr* Parser::primary() {
        Expr* node;

        if (Key* key = eat<Key>()) {
            if (Unary::Op op = unop(key); op != Unary::INVALID) {
                node = new Unary(op, primary());
            } else if (key->key == Key::LPAREN) {
                node = expr();
                expect<Key>(Key::RPAREN);
            } else if (key->key == Key::COERCE) {
                expect<Key>(Key::BEGIN);
                Type* to = type();
                expect<Key>(Key::END);
                expect<Key>(Key::LPAREN);
                Expr* it = expr();
                expect<Key>(Key::RPAREN);
                node = new Coerce(to, it);
            } else if (key->key == Key::TRUE || key->key == Key::FALSE) {
                node = new BoolConst(key);
            } else {
                node = nullptr;
            }
        } else if (dynamic_cast<Ident*>(peek())) {
            node = new NameExpr(qual());
        } else if (Int* i = eat<Int>(); i) {
            node = new IntConst(i);
        } else if (String* s = eat<String>(); s) {
            node = new StrConst(s);
        } else if (Char* c = eat<Char>(); c) {
            node = new CharConst(c);
        } else {
            node = nullptr;
        }

        while (true) {
            if (Key* lsquare = eat<Key>(Key::LSQUARE); lsquare) {
                node = new Subscript(node, expr());
                expect<Key>(Key::RSQUARE);
            } else if (Key* lparen = eat<Key>(Key::LPAREN); lparen) {
                node = new Call(node, gather<FunctionParam>(Key::COMMA, Key::RPAREN, [](Parser* self) {
                    return self->funcParam();
                }));
            } else if (Key* dot = eat<Key>(Key::DOT); dot) {
                node = new Dot(node, expect<Ident>(), false);
            } else if (Key* arrow = eat<Key>(Key::ARROW); arrow) {
                node = new Dot(node, expect<Ident>(), true);
            } else {
                break;
            }
        }

        return node;
    }

    Name* Parser::name() {
        Ident* id = expect<Ident>();
        if (Key* begin = eat<Key>(Key::BEGIN); begin) {
            vector<Type*> params = collect<Type>(Key::COMMA, [](Parser* self) {
                return self->type();
            });
            expect<Key>(Key::END);

            return new Name(id, params);
        } else {
            return new Name(id);
        }
    }

    Qual* Parser::qual() {
        return new Qual(collect<Name>(Key::COLON2, [](Parser* self) {
            return self->name();
        }));
    }

    Decl* Parser::include() {
        if (!eat<Key>(Key::USING)) {
            return nullptr;
        }
        vector<Ident*> path = collect<Ident>(Key::COLON2, [](Parser* self) {
            return self->expect<Ident>();
        });
        Decl* out;
        if (eat<Key>(Key::ASSIGN)) {
            // actually an alias
            out = new Alias(path[0], type());
        } else if (eat<Key>(Key::LPAREN)) {
            if (eat<Key>(Key::DOT3)) {
                out = new Import(path, {});
                expect<Key>(Key::RPAREN);
            } else {
                out = new Import(path, gather<Ident>(Key::COMMA, Key::RPAREN, [](Parser* self) {
                    return self->expect<Ident>();
                }));
            }
        } else {
            out = new Import(path);
        }

        expect<Key>(Key::SEMI);
        return out;
    }

    Decl* Parser::decl() {
        if (Alias* a = alias(); a) {
            return a;
        } 

        if (Var* v = var(); v) {
            return v;
        } 

        return nullptr;
    }

    Alias* Parser::alias() {
        if (!eat<Key>(Key::USING)) {
            return nullptr;
        }
        Ident* name = expect<Ident>();
        expect<Key>(Key::ASSIGN);
        Type* it = type();
        expect<Key>(Key::SEMI);

        return new Alias(name, it);
    }

    Var* Parser::var() {
        bool mut;
        if (eat<Key>(Key::LET)) {
            mut = false;
        } else if (eat<Key>(Key::VAR)) {
            mut = true;
        } else {
            return nullptr;
        }

        vector<VarName*> names;
        if (eat<Key>(Key::LSQUARE)) {
            names = gather<VarName>(Key::COMMA, Key::RSQUARE, [](Parser* self) {
                return self->varName();
            });
        } else {
            names = { varName() };
        }

        Var* out = new Var(names, eat<Key>(Key::ASSIGN) ? expr() : nullptr, mut);
        expect<Key>(Key::SEMI);
        return out;
    }

    VarName* Parser::varName() {
        Ident* n = expect<Ident>();
        return new VarName(n, eat<Key>(Key::COLON) ? type() : nullptr);
    }

    Struct* Parser::struct_() {
        if (!eat<Key>(Key::STRUCT)) {
            return nullptr;
        }

        Ident* name = expect<Ident>();

        expect<Key>(Key::LBRACE);

        vector<Decl*> body;
        Decl* it = decl();
        while (it != nullptr) {
            body.push_back(it);
            it = decl();
        }

        expect<Key>(Key::RBRACE);

        return new Struct(name, body);
    }

    Union* Parser::union_() {
        if (!eat<Key>(Key::UNION)) {
            return nullptr;
        }

        return nullptr;
    }

    Decl* Parser::enum_() {
        if (!eat<Key>(Key::ENUM)) {
            return nullptr;
        }

        if (eat<Key>(Key::UNION)) {
            // tagged union
        }

        return nullptr;
    }

    ///
    /// parsing helper functions
    /// 

    Token* Parser::next() {
        Token* token;
        
        if (ahead != nullptr) {
            token = ahead;
            ahead = nullptr;
        } else {
            token = lexer->read();
        }   

        return token;
    }

    Token* Parser::peek() {
        if (ahead == nullptr) {
            ahead = lexer->read();
        }

        return ahead;
    }
}