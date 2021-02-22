#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <iostream>
#include <tinyutf8/tinyutf8.h>

namespace cthulhu {
    using namespace std;

    namespace utf8 = tiny_utf8;

    struct Stream;
    struct Lexer;
    struct Location;
    struct Range;
    struct Token;
    struct Ident;
    struct Key;
    struct Int;
    struct String;
    struct End;

    struct Node;
    struct Expr;
    struct Type;
    struct Name;
    struct Qualified;

    static constexpr char32_t END = char32_t(-1);

    struct StreamHandle {
        virtual ~StreamHandle() {}
        virtual char32_t next() = 0;
    };

    struct Stream {
        Stream(StreamHandle* handle);

        char32_t next();
        char32_t peek();
    private:
        StreamHandle* handle;
        char32_t lookahead;
    };

    struct Location {
        Lexer* lexer;
        size_t offset;
        size_t line;
        size_t column;

        Location(Lexer* lexer, size_t offset, size_t line, size_t column);

        Range* to(Location other);
    };

    struct Range : Location {
        size_t length;

        Range(Lexer* lexer, size_t offset, size_t line, size_t column, size_t length);
    };

    struct Lexer {
        Lexer(Stream stream);

        Token* read();

        utf8::string slice(Range* range);

    private:
        utf8::string collect(char32_t start, bool(*filter)(char32_t));

        char32_t skip();
        char32_t next();
        char32_t peek();
        bool eat(char32_t c);

        Stream stream;
        Location here;
        utf8::string text;
        int depth;
    };

    struct Token {
        Range* range;

        virtual ~Token() {}
    };

    struct Ident : Token {
        Ident(utf8::string id);

        bool operator==(const Ident& other) const;

        utf8::string get() const { return ident; }

    private:
        utf8::string ident;
    };

    struct Key : Token {
#define KEY(id, _) id,
#define OP(id, _) id,
        enum Word { 
#include "keys.inc"
            INVALID
        } key;

        Key(Word word);

        bool operator==(const Key& other) const;
        bool equals(Word word) const { return key == word; }
    };

    struct Int : Token {
        Int(uint64_t num, utf8::string str = u8"");

        bool operator==(const Int& other) const;

    private:
        uint64_t num;
        utf8::string suffix;
    };

    struct String : Token {
        String(utf8::string str);

        bool operator==(const String& other) const;

    private:
        utf8::string str;
    };

    struct End : Token {
        End() { }

        bool operator==(const End&) const { return true; }
    };

    struct Error : Token {

    };

    struct Parser {
        Parser(Lexer* lexer);

        Type* type();
        Expr* expr();

        Name* name();
        Qualified* qualified();

        template<typename T, typename... A>
        T* eat(A&&... args) {
            Token* token = next();
            if (T* it = dynamic_cast<T*>(token); it) {
                
                if constexpr (sizeof...(A) > 0) {
                    if (!(*it == T(args...))) {
                        return nullptr;
                    }
                }

                return it;
            }
                
            ahead = token;
            return nullptr;
        }

        template<typename T, typename... A>
        T* expect(A&&... args) {
            if (T* it = eat<T>(args...); it == nullptr) {
                printf("expected %s but got %s instead\n", typeid(T).name(), typeid(*ahead).name());
                exit(1);
            } else {
                return it;
            }
        }

        Token* next();
        Token* peek();

        template<typename T>
        vector<T*> collect(Key::Word sep, T*(*func)(Parser*)) {
            vector<T*> out;

            do { 
                out.push_back(func(this)); 
            } while (eat<Key>(sep) != nullptr);

            return out;
        }

        template<typename T>
        vector<T*> gather(Key::Word sep, Key::Word until, T*(*func)(Parser*)) {
            vector<T*> out;

            while (true) {
                if (eat<Key>(until) == nullptr) {
                    out.push_back(func(this));
                    expect<Key>(sep);
                } else {
                    break;
                }
            }

            return out;
        }

        Lexer* lexer;
        Token* ahead;
    };

    struct Node {
        virtual ~Node() { }

        virtual utf8::string repr() const = 0;

        vector<Token*> tokens;
    };

    struct Expr : Node {

    };

    // type: qualified | pointer | array | closure ;
    // types: type (`,` type)* ;
    struct Type : Node {

    };

    // pointer: `*` type ;
    struct Pointer : Type {
        Pointer(Type* type)
            : type(type)
        { }

        virtual utf8::string repr() const override {
            return "*" + type->repr();
        }

        Type* type;
    };

    // name: ID (`!<` types `>`)
    struct Name : Type {
        Name(Ident* ident) 
            : name(ident->get())
            , params({}) 
        { }

        virtual utf8::string repr() const override {
            utf8::string out = name;

            if (params.size() != 0) {
                out += "!<";
                for (size_t i = 0; i < params.size(); i++) {
                    if (i) {
                        out += ", ";
                    }

                    out += params[i]->repr();
                }
                out += ">";
            }

            return out;
        }

        utf8::string name;
        vector<Type*> params;
    };

    // qualified: name (`::` type)* ;
    struct Qualified : Type {
        Qualified(vector<Name*> names)
            : names(names)
        { }

        virtual utf8::string repr() const override {
            utf8::string out;

            for (size_t i = 0; i < names.size(); i++) {
                if (i) {
                    out += "::";
                }

                out += names[i]->repr();
            }

            return out;
        }

        vector<Name*> names;
    };

    // array: `[` type (`:` expr)? `]` ;
    struct Array : Type {
        Array(Type* type, Expr* size)
            : type(type)
            , size(size)
        { }

        virtual utf8::string repr() const override {
            return "[" + type->repr() + "]";
        }

        Type* type;
        Expr* size;
    };

    // closure: type `(` types? `)` ;
    struct Closure : Type {
        Closure(Type* result, vector<Type*> params)
            : result(result)
            , params(params)
        { }

        virtual utf8::string repr() const override {
            utf8::string out = result->repr() + "(";

            for (size_t i = 0; i < params.size(); i++) {
                if (i) {
                    out += ", ";
                }

                out += params[i]->repr();
            }

            return out + ")";
        }

        Type* result;
        vector<Type*> params;
    };
}
