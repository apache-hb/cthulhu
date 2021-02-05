#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <istream>

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
            int c = skip();
            auto start = here;
            Token* out;

            if (isident1(c)) {
                out = ident(c);
            } else if (c == '0') {
                out = digit0();
            } else if (isdigit(c)) {
                out = digit(c);
            } else {
                out = symbol(c);
            }

            start.length = here.offset - start.offset;
            out->range = start;

            return out;
        }

    private:
        Token* ident(int c) {
            std::string str = {(char)c};
            while (isident2(peek())) {
                str.push_back((char)next());
            }

            return nullptr;
        }

        Token* digit0() {
            return nullptr;
        }

        Token* digit(int c) {
            std::string str = {(char)c};
            
            while (isdigit(peek())) {
                str.push_back((char)next());
            }

            return nullptr;
        }

        Token* symbol(int c) {
            switch (c) {
            case '(':
            default: return nullptr;
            }
            return nullptr;
        }

        static bool isident1(int c) {
            return isalpha(c) || c == '_';
        }

        static bool isident2(int c) {
            return isalnum(c) || c == '_';
        }

        int skip() {
            int c = next();
            
            while (isspace(c)) {
                c = next();
            }

            return c;
        }

        int next() {
            int c = in->get();
            if (c == '\n') {
                here.line += 1;
                here.column = 0;
            } else {
                here.column += 1;
            }

            here.offset += 1;

            return c;
        }

        int peek() {
            return in->peek();
        }
        
        Range here;
        std::string buf;
        std::istream* in;
    };
}

namespace ast {
    struct Node {
        // the token that produced this node
        lex::Token* parent;

        virtual std::string repr() const { 
            return "Node(" + parent->repr() + ")"; 
        }
        
        virtual bool validate(std::string& err) const {
            if (parent == nullptr) {
                err = "parent token is null";
                return false;
            }

            return parent->validate(err);
        }
    };

    struct Ident : Node {
        virtual std::string repr() const override { 
            return "Ident(" + parent->repr() + ")"; 
        }
    };

    struct Path : Node {
        std::vector<Node*> parts;

        virtual std::string repr() const override { 
            std::string out = "Path(";
            for (size_t i = 0; i < parts.size(); i++) {
                if (i != 0) {
                    out += ", ";
                }
                out += parts[i]->repr();
            }
            out += ")";

            return out;
        }

        virtual bool validate(std::string& err) const override {
            if (parts.empty()) {
                err = "path is empty";
                return false;
            }

            for (Node* node : parts) {
                if (!node->validate(err)) {
                    return false;
                }
            }

            return true;
        }
    };
}

struct Parser {

};
