#pragma once

#include "lex.h"
#include "ast.h"

struct Parser {
    Parser(Lexer l)
        : lex(l)
    { }

    std::vector<std::string> _name() {
        std::vector<std::string> out;
        do { 
            out.push_back(expect<Ident>().ident);
        } while(consume(Key::COLON));
        return out;
    }

    Import _import() {
        Import ret = { _name() };
        if(consume(Key::ARROW))
            ret.alias = expect<Ident>().ident;
        
        linefeed();
        return ret;
    }

    Program program() {
        Program prog;
        while(consume(Key::IMPORT)) {
            prog.deps.push_back(_import());
        }

        return prog;
    }

    bool consume(Key::key_t key) {
        auto temp = tok;
        if(temp.has_value())
            temp = lex.next();
        
        if(auto k = temp->as<Key>(); k && k->key == key) {
            tok = std::nullopt;
            return true;
        } else {
            tok = temp;
            return false;
        }
    }

    void linefeed() {
        auto temp = get();
        if(auto key = temp.as<Key>(); key->key == Key::SEMICOLON || key->key == Key::NEWLINE) {
            return;
        }

        printf("expected linefeed\n");    
    }

    template<typename T>
    T expect() {
        if(tok) {
            if(auto t = tok->as<T>()) {
                tok = std::nullopt;
                return *t;
            }
        }

        printf("unexpected token\n");
        return T();
    }

    Token get() {
        if(tok.has_value()) {
            auto temp = tok.value();
            tok = std::nullopt;
            return temp;
        } else {
            return lex.next();
        }
    }

    std::optional<Token> tok = std::nullopt;
    Lexer lex;
};