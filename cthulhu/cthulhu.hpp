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
    struct Qual;

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
        Lexer(Stream stream, utf8::string name = u8"input")
            : name(name)
            , stream(stream)
            , here(Location(this, 0, 0, 0))
        { }

        Token* read();

        utf8::string slice(Range* range);
        utf8::string line(size_t it);

        const utf8::string name;

    private:
        utf8::string collect(char32_t start, bool(*filter)(char32_t));

        char32_t skip();
        char32_t next();
        char32_t peek();
        char32_t escape();
        bool eat(char32_t c);

        Stream stream;
        Location here;
        utf8::string text = "";
        int depth = 0;
    };

    struct Token {
        Range* range;

        virtual ~Token() {}

        virtual utf8::string repr() const {
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

        utf8::string underline(utf8::string error = "", utf8::string note = "") const {
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

    struct Node {
        virtual ~Node() { }

        virtual utf8::string repr() const { return u8"Node()"; }
    };

    struct Expr : Node {

    };

    struct IntConst : Expr {
        Int* num;

        IntConst(Int* i)
            : num(i)
        { }

        virtual utf8::string repr() const override { 
            utf8::string out = "Int(";
            out += std::to_string(num->get());
            out += num->suf();
            return out + ")";
        }
    };

    struct StrConst : Expr {
        utf8::string val;

        virtual utf8::string repr() const override { 
            utf8::string out = "String(\"";
            out += val;
            return out + "\")";
        }
    };

    struct BoolConst : Expr {
        bool val;

        virtual utf8::string repr() const override { 
            utf8::string out = "Bool(";
            out += std::to_string(val);
            return out + ")";
        }
    };

    struct CharConst : Expr {
        char32_t val;

        virtual utf8::string repr() const override { 
            utf8::string out = "Char(";
            out += val;
            return out + ")";
        }
    };

    struct Ternary : Expr {
        Expr* cond;
        Expr* lhs;
        Expr* rhs;

        Ternary(Expr* cond, Expr* lhs, Expr* rhs)
            : cond(cond)
            , lhs(lhs)
            , rhs(rhs)
        { }

        virtual utf8::string repr() const override { 
            utf8::string out = "Ternary(cond=" + cond->repr();
            if (lhs) {
                out += ",lhs=" + lhs->repr();
            }
            out += ",rhs=" + rhs->repr() + ")";
            return out;
        }
    };

    struct Unary : Expr {
        enum Op {
            INVALID,
            NOT, // !
            FLIP, // ~
            POS, // + 
            NEG, // -
            DEREF, // *
            REF // &
        };

        Op op;
        Expr* expr;

        Unary(Op op, Expr* expr)
            : op(op)
            , expr(expr)
        { }

        virtual utf8::string repr() const override { 
            utf8::string out = "Unary(op=";
            switch (op) {
            case NOT: out += "!"; break;
            case FLIP: out += "~"; break;
            case POS: out += "+"; break;
            case NEG: out += "-"; break;
            case DEREF: out += "*"; break;
            case REF: out += "&"; break;
            }
            return out + ",expr=" + expr->repr() + ")";
        }
    };

    struct Binary : Expr {
        enum Op {
            INVALID,
            ADD, // +
            SUB, // -
            MUL, // *
            DIV, // /
            MOD, // %
            BITAND, // &
            BITOR, // |
            BITXOR, // ^
            AND, // &&
            OR, // ||
            SHL, // <<
            SHR, // >>
            LT, // <
            LTE, // <=
            GT, // >
            GTE, // >=
            EQ, // ==
            NEQ // !=
        };

        Op op;
        Expr* lhs;
        Expr* rhs;

        Binary(Op op, Expr* lhs, Expr* rhs)
            : op(op)
            , lhs(lhs)
            , rhs(rhs)
        { }

        virtual utf8::string repr() const override { 
            utf8::string out = "Binary(op=";
            switch (op) {
            case ADD: out += "+"; break;
            case SUB: out += "-"; break;
            case MUL: out += "*"; break;
            case DIV: out += "/"; break;
            case MOD: out += "%"; break;
            case BITAND: out += "&"; break;
            case BITOR: out += "|"; break;
            case BITXOR: out += "^"; break;
            case AND: out += "&&"; break;
            case OR: out += "||"; break;
            case SHL: out += "<<"; break;
            case SHR: out += ">>"; break;
            case LT: out += "<"; break;
            case LTE: out += "<="; break;
            case GT: out += ">"; break;
            case GTE: out += ">="; break;
            case EQ: out += "=="; break;
            case NEQ: out += "!="; break;
            }
            return out + ",lhs=" + lhs->repr() + ",rhs=" + rhs->repr() + ")";
        }
    };

    struct Type : Node {

    };

    struct Name : Type {
        Name(Ident* tok)
            : name(tok->get())
        { }

        Name(Ident* tok, vector<Type*> params)
            : params(params)
            , name(tok->get())
        { }

        vector<Type*> params;

        utf8::string name;

        virtual utf8::string repr() const override {
            utf8::string out = "Name(" + name;

            if (!params.empty()) {
                out += ",params=(";
                for (size_t i = 0; i < params.size(); i++) {
                    if (i) {
                        out += ",";
                    }
                    out += params[i]->repr();
                }
                out += ")";
            }

            return out + ")";
        }
    };

    struct Qual : Type {
        Qual(vector<Name*> names)
            : names(names)
        { }

        vector<Name*> names;
        
        virtual utf8::string repr() const override {
            utf8::string out = "Qual(";

            for (size_t i = 0; i < names.size(); i++) {
                if (i) {
                    out += ",";
                }
                out += names[i]->repr();
            }

            return out + ")";
        }
    };

    struct Pointer : Type {
        Pointer(Type* type)
            : type(type)
        { }

        Type* type;

        virtual utf8::string repr() const override {
            return "Pointer(" + type->repr() + ")";
        }
    };

    struct Array : Type {
        Array(Type* type, Expr* size)
            : type(type)
            , size(size)
        { }

        Type* type;
        Expr* size = nullptr;

        virtual utf8::string repr() const override {
            utf8::string out = "Array(of=" + type->repr();

            if (size) {
                out += ",size=" + size->repr();
            }

            return out + ")";
        }
    };

    struct Closure : Type {
        Type* type;
        vector<Type*> params;

        Closure(Type* type, vector<Type*> params)
            : type(type)
            , params(params)
        { }

        virtual utf8::string repr() const override {
            utf8::string out = "Closure(result=" + type->repr();

            if (!params.empty()) {
                out += ",args=(";
                for (size_t i = 0; i < params.size(); i++) {
                    if (i) {
                        out += ",";
                    }
                    out += params[i]->repr();
                }
                out += ")";
            }

            return out + ")";
        }
    };

    struct Coerce : Expr {
        Type* type;
        Expr* expr;

        Coerce(Type* type, Expr* expr)
            : type(type)
            , expr(expr)
        { }

        virtual utf8::string repr() const override { 
            utf8::string out = "Coerce(";
            out += type->repr() + "," + expr->repr();
            return out + ")";
        }
    };

    struct NameExpr : Expr {
        Qual* name;

        NameExpr(Qual* qual)
            : name(qual)
        { }
    };

    struct Parser {
        Parser(Lexer* lexer)
            : lexer(lexer)
        { }

        Type* type();
        Array* array();
        Name* name();
        Qual* qual();

        Expr* expr();
        Expr* binary(int mprec);
        Expr* primary();

        template<typename T, typename... A>
        T* eat(A&&... args) {
            Token* token = next();

            if (T* it = dynamic_cast<T*>(token); it != nullptr) {
                if constexpr (sizeof...(A) > 0) {
                    if (!(*it == T(args...))) {
                        ahead = token;
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

                if constexpr (sizeof...(A) > 0) {
                    printf("expected %s but got %s instead\n", T(args...).repr().c_str(), ahead->repr().c_str());
                } else {
                    printf("expected %s but got %s instead\n", typeid(T).name(), ahead->repr().c_str());
                }

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

            while (eat<Key>(until) == nullptr) {
                out.push_back(func(this));

                if (eat<Key>(until) == nullptr) {
                    expect<Key>(sep);
                } else {
                    break;
                }
            }

            return out;
        }

        Lexer* lexer;
        Token* ahead = nullptr;
    };
}
