#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <istream>
#include <limits>
#include <unordered_map>

namespace lex {
    using size_t = std::size_t;
    
    struct Range {
        size_t offset;
        size_t length;
        size_t column;
        size_t line;
    };

#define ASSERT_TYPE(type) if (!is(type)) { err = "incorrect token type"; return false; }

    struct Token {
        enum Type { STRING, IDENT, KEYWORD, INT, CHAR, END };

        virtual ~Token() { }

        // return a string for debugging
        virtual std::string repr() const = 0;

        // return a pretty string
        virtual std::string string() const = 0;

        virtual bool validate(std::string& err) const { 
            err = "Token directly instanciated";
            return false;
        }

        bool is(Type other) const { return type == other; }

    protected:
        Token(Type in) : type(in), range({}) { }

        friend struct Lexer;

    private:
        Type type;
        Range range;
    };

    struct String : Token {
        String(std::string s) : Token(Token::STRING), str(s) { }

        virtual ~String() override { }
        virtual std::string repr() const override { return "String(" + str + ")"; }
        virtual std::string string() const override { return "\"" + str + "\""; }

        virtual bool validate(std::string& err) const override { 
            ASSERT_TYPE(Token::STRING);
            return true; 
        }

    private:
        std::string str;
    };

    struct Ident : Token {
        Ident(std::string name) : Token(Token::IDENT), id(name) { }

        virtual ~Ident() override { }
        virtual std::string repr() const override { return "Ident(" + id + ")"; }
        virtual std::string string() const override { return id; }

        virtual bool validate(std::string& err) const override {
            ASSERT_TYPE(Token::IDENT);
            if (id.empty()) {
                err = "ident is empty";
                return false;
            } else {
                return true; 
            }
        }

    private:
        std::string id;
    };

    struct Keyword : Token {
        enum Key : int { 
            INVALID,
#define KEY(id, str) id,
#define OP(id, str) id,
#include "keys.inc"
        };
        Keyword(Key k) : Token(Token::KEYWORD), key(k) { }

        static std::string str(Key k) {
#define KEY(id, str) case id: return str;
#define OP(id, str) case id: return str;
            switch (k) {
#include "keys.inc"
                default: 
                    return "InvalidKey(" + std::to_string((int)k) + ")";
            }
        }

        virtual ~Keyword() override { }
        virtual std::string repr() const override { return "Key(" + str(key) + ")"; }
        virtual std::string string() const override { return str(key); }

        bool eq(Key k) const { return k == key; }

        virtual bool validate(std::string& err) const override { 
            ASSERT_TYPE(Token::KEYWORD);
            if (key == INVALID) {
                err = "keyword was invalid";
                return true;
            } else {
                return false; 
            }
        }

    private:
        Key key;
    };

    struct Int : Token {
        Int(size_t num) : Token(Token::INT), n(num) { }

        virtual ~Int() override { }
        virtual std::string repr() const override { return "Int(" + std::to_string(n) + ")"; }
        virtual std::string string() const override { return std::to_string(n); }

        virtual bool validate(std::string& err) const override { 
            ASSERT_TYPE(Token::INT);
            return true; 
        }

    private:
        size_t n;
    };

    struct Char : Token {
        Char(size_t code) : Token(Token::CHAR), c(code) { }

        virtual ~Char() override { }
        virtual std::string repr() const override { return "Char(" + std::to_string(c) + ")"; }
        virtual std::string string() const override { return std::to_string(c); }

        virtual bool validate(std::string& err) const override { 
            ASSERT_TYPE(Token::CHAR);
            return true; 
        }

    private:
        size_t c;
    };

    struct End : Token {
        End() : Token(Token::END) { }

        virtual ~End() override { }
        virtual std::string repr() const override { return "End"; }
        virtual std::string string() const override { return "\0"; }

        virtual bool validate(std::string& err) const override { 
            ASSERT_TYPE(Token::END);
            return true; 
        }
    };

    struct Lexer {
        Lexer(std::istream *source) 
            : here({})
            , in(source) 
        { }

        Token* read() {
            char c = skip(iswhite);
            auto start = here;
            Token* tok;

            if (isident1(c)) {
                tok = get<Token*>(collect(c, isident2),
                    [](auto k) { return new Keyword(k); },
                    [](auto id) { return new Ident(id); }
                );
            } else if (c == '0') {
                tok = digit0();
            } else if (isdigit(c)) {
                tok = base10(c);
            } else {
                tok = symbol(c);
            }

            start.length = here.offset - start.offset;
            tok->range = start;

            return tok;
        }

    private:
        Token* digit0() {
            switch (next()) {
            case 'x': return base16();
            case 'b': return base2();
            }
            return nullptr;
        }

        Token* base16() {
            return nullptr;
        }

        Token* base2() {
            return nullptr;
        }

        Token* base10(char c) {
            (void)c;
            return nullptr;
        }

        Token* symbol(char c) {
            switch (c) {
            case '(': return new Keyword(Keyword::LPAREN);
            case ')': return new Keyword(Keyword::RPAREN);
            case '[': return new Keyword(Keyword::LSQUARE);
            case ']': return new Keyword(Keyword::RSQUARE);
            case '{': return new Keyword(Keyword::LBRACE);
            case '}': return new Keyword(Keyword::RBRACE);
            case '@': return new Keyword(Keyword::AT);
            case ',': return new Keyword(Keyword::COMMA);
            
            case '<': {
                if (eat('<')) {
                    return new Keyword(eat('=') ? Keyword::SHLEQ : Keyword::SHL);
                }

                return new Keyword(eat('=') ? Keyword::LTE : Keyword::LT);
            }

            case '>': {
                if (depth > 0) {
                    depth--;
                    return new Keyword(Keyword::END);
                }

                if (eat('>')) {
                    return new Keyword(eat('=') ? Keyword::SHREQ : Keyword::SHR);
                }

                return new Keyword(eat('=') ? Keyword::GTE : Keyword::GT);
            }

            case '!': {
                if (eat('<')) {
                    depth++;
                    return new Keyword(Keyword::BEGIN);
                }

                return new Keyword(eat('=') ? Keyword::NEQ : Keyword::NOT);
            }

            case '=': {
                return new Keyword(eat('=') ? Keyword::EQ : Keyword::ASSIGN);
            }

            case '&': {
                if (eat('&')) {
                    return new Keyword(Keyword::AND);
                } else {
                    return new Keyword(eat('=') ? Keyword::BITANDEQ : Keyword::BITAND);
                }
            }

            case '|': {
                if (eat('|')) {
                    return new Keyword(Keyword::OR);
                } else {
                    return new Keyword(eat('=') ? Keyword::BITOREQ : Keyword::BITOR);
                }
            }

            case '^': return new Keyword(eat('=') ? Keyword::BITXOREQ : Keyword::BITXOR);
            case '~': return new Keyword(Keyword::BITNOT);
            case '+': return new Keyword(eat('=') ? Keyword::ADDEQ : Keyword::ADD);
            case '-': {
                if (eat('>')) {
                    return new Keyword(Keyword::ARROW);
                }
                return new Keyword(eat('=') ? Keyword::SUBEQ : Keyword::SUB);
            }
            case '*': return new Keyword(eat('=') ? Keyword::MULEQ : Keyword::MUL);
            case '/': return new Keyword(eat('=') ? Keyword::DIVEQ : Keyword::DIV);
            case '%': return new Keyword(eat('=') ? Keyword::MODEQ : Keyword::MOD);
            case ':': return new Keyword(eat(':') ? Keyword::COLON2 : Keyword::COLON);
            case ';': return new Keyword(Keyword::SEMI);

            case '.': {
                if (eat('.')) {
                    if (eat('.')) {
                        return new Keyword(Keyword::DOT3);
                    }

                    return new Keyword(Keyword::DOT2);
                }

                return new Keyword(Keyword::DOT);
            }
            default:
                return nullptr;
            }
        }

        static bool isident1(char c) {
            return isalpha(c) || c == '_';
        }

        static bool isident2(char c) {
            return isalnum(c) || c == '_';
        }

        static bool iswhite(char c) {
            return isspace(c);
        }

        static inline std::unordered_map<std::string, Keyword::Key> keys = {
#define KEY(id, str) { str, Keyword::id },
#include "keys.inc"
        };

        bool eat(char c) {
            if (peek() == c) {
                next();
                return true;
            }
            return false;
        }

        template<typename T, typename F1, typename F2>
        T get(std::string id, F1&& f1, F2&& f2) {
            auto iter = keys.find(id);
            if (iter != keys.end()) {
                return f1(iter->second);
            } else {
                return f2(id);
            }
        }

        template<typename F>
        std::string collect(char c, F&& func) {
            std::string str = {c};
            
            while (func(peek())) {
                str += next();
            }

            return str;
        }

        template<typename F>
        char skip(F&& func) {
            char c = next();
            
            while (func(c)) {
                c = next();
            }

            return c;
        }

        using limit = std::numeric_limits<char>;

        static void verify_range(int c) {
            if (limit::min() > c || c > limit::max()) {
                throw std::out_of_range("c was out of range");
            }
        }

        char next() {
            int c = in->get();
            verify_range(c);

            if (c == '\n') {
                here.line += 1;
                here.column = 0;
            } else {
                here.column += 1;
            }

            here.offset += 1;

            return (char)c;
        }

        char peek() {
            int c = in->peek();
            verify_range(c);
            return (char)c;
        }
        
        Range here;
        std::string buf;
        std::istream* in;
        int depth = 0;
    };
}

namespace ast {
    struct Node {
        // the token that produced this node
        lex::Token* parent;

        Node(lex::Token* tok) : parent(tok) { }

        virtual std::string repr() const { 
            return "Node(" + parent->repr() + ")"; 
        }
    };

    struct Error : Node {
        std::string msg;

        Error(lex::Token* tok, std::string m) : Node(tok), msg(m) { }

        virtual std::string repr() const override {
            return "Error(" + Node::repr() + "): " + msg;
        }
    };

    struct Ident : Node {
        Ident(lex::Token* tok) : Node(tok) { }
    };

    struct Expr : Node {

    };

    struct Type : Node {
        Type(lex::Token* tok) : Node(tok) { }
    };

    struct RefType : Type {
        Node* to;
    };

    struct PtrType : Type {
        PtrType(lex::Token* p, Node* t) : Type(p), to(t) { }
        Node* to;
    };

    struct VarType : Type {
        Type* to;
    };

    struct NameType : Type {
        NameType(Node* n) : name(n) { }
        Node* name;
        Node* types;
    };

    struct QualType : Type {
        NameType* body;
    };

    struct FuncType : Type {
        Type* result;
        std::vector<Type*> args;
    };

    struct ArrType : Type {
        Type* of;
        Expr* size;
    };
}

struct Parser {
    Parser(lex::Lexer* l) {
        in = l;
        tok = l->read();
    }

    ast::Node* parseNameType() {
        if (auto* id = parseIdent()) {
            return new ast::NameType(id, parseTypeArgs());
        }

        return nullptr;
    }

    ast::Node* parseIdent() {
        auto* temp = peek();
        if (temp->is(lex::Token::IDENT)) {
            return new ast::Ident(temp);
        }

        return nullptr;
    }

    ast::Node* parseTypeArgs() {
        if (auto* it = eat(lex::Keyword::BEGIN)) {
            return new ast::TypeArgs(it, collect(parseType, lex::Keyword::COMMA), eat(lex::Keyword::END));
        }

        return nullptr;
    }

private:
    template<typename F>
    std::vector<ast::Node*> collect(F&& func, lex::Keyword::Key k) {
        std::vector<ast::Node*> out;
        do { out.push_back(func()); } while (eat(k));
        return out;
    }

    lex::Token* eat(lex::Keyword::Key k) {
        auto* temp = peek();
        if (temp->is(lex::Token::KEYWORD) && ((lex::Keyword*)temp)->eq(k)) {
            return next();
        }
        return nullptr;
    }

    lex::Token* next() {
        auto* t = tok;
        tok = in->read();
        return t;
    }

    lex::Token* peek() {
        return tok;
    }

    lex::Token* tok;
    lex::Lexer* in;
};
