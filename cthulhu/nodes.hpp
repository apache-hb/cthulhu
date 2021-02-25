#pragma once

#include "debug.hpp"

namespace cthulhu {
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

    struct Token {
        Range* range;

        virtual ~Token() {}

        virtual utf8::string repr() const;

        utf8::string underline(utf8::string error = "", utf8::string note = "") const;
    };

    struct Ident : Token {
        Ident(utf8::string id)
            : ident(id)
        { }

        bool operator==(const Ident& other) const { return ident == other.ident; }

        utf8::string get() const { return ident; }

        virtual utf8::string repr() const override {
            return utf8::string("Ident(") + ident + ")";
        }

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

        Key(Word word)
            : key(word)
        { }

        bool operator==(const Key& other) const { return key == other.key; }

        virtual utf8::string repr() const override {
#define KEY(id, str) case id: return "Key(`" str "`)";
#define OP(id, str) case id: return "Key(`" str "`)";
            switch (key) {
#include "keys.inc"
            default: return "Key(`INVALID`)";
            }
        }
    };

    struct Int : Token {
        Int(uint64_t num, utf8::string str = u8"")
            : num(num)
            , suffix(str)
        { }

        bool operator==(const Int& other) const { return num == other.num && suffix == other.suffix; }

        virtual utf8::string repr() const override {
            utf8::string out = "Int(" + std::to_string(num);

            if (!suffix.empty()) {
                out += ",suffix=" + suffix;
            }

            out += ")";

            return out;
        }

        uint64_t get() const { return num; }
        utf8::string suf() const { return suffix; }

    private:
        uint64_t num;
        utf8::string suffix;
    };

    struct String : Token {
        String(utf8::string str)
            : str(str)
        { }

        bool operator==(const String& other) const { return str == other.str; }

        virtual utf8::string repr() const override {
            return utf8::string("String(") + str + ")";
        }

        utf8::string get() const { return str; }

    private:
        utf8::string str;
    };

    struct Char : Token {
        Char(char32_t c)
            : c(c)
        { }

        bool operator==(const Char& other) const { return c == other.c; }
    
        virtual utf8::string repr() const override {
            return utf8::string("Char(") + std::to_string(c) + ")";
        }

        char32_t get() const { return c; }
    private:
        char32_t c;
    };

    struct End : Token {
        End() { }

        bool operator==(const End&) const { return true; }

        virtual utf8::string repr() const override {
            return "End()";
        }
    };
}
