#pragma once

#include <iostream>

#include "lexer.hpp"
#include "ast.hpp"

//
// main parser class
//

namespace cthulhu {
    struct Parser {
        Parser(Lexer* lexer)
            : lexer(lexer)
        { }

        Unit* unit_();

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

        Lexer* lexer;
        Token* ahead = nullptr;
    };
}
