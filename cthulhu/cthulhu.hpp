#pragma once

#include <iostream>

#include "lexer.hpp"
#include "ast.hpp"

namespace cthulhu {
    struct Parser {
        Parser(Lexer* lexer)
            : lexer(lexer)
        { }

        Unit* unit();

        Type* type();
        Array* array();
        Name* name();
        Qual* qual();

        Expr* expr();
        Expr* binary(int mprec);
        Expr* primary();

        Decl* include();
        Decl* decl();
        Alias* alias();
        Var* var();
        VarName* varName();
        FunctionParam* funcParam();

        Struct* struct_();
        Union* union_();
        Decl* enum_();

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
