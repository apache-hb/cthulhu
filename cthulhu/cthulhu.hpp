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

        Function* function();
        FunctionArg* funcArg();

        Stmt* stmt();
        Stmt* compound();
        Stmt* while_();
        Stmt* return_();
        Stmt* for_();
        Stmt* if_();
        Stmt* switch_();

        Struct* struct_();
        Union* union_();
        Decl* enum_();
        Template* template_();
        TemplateParam* templateParam();

        Decorator* decorator();

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

        template<typename T, typename F>
        vector<T*> collect(Key::Word sep, F&& func) {
            vector<T*> out;

            do {
                out.push_back(func()); 
            } while (eat<Key>(sep) != nullptr);

            return out;
        }

        template<typename T, typename F>
        vector<T*> gather(Key::Word sep, Key::Word until, F&& func) {
            vector<T*> out;

            while (eat<Key>(until) == nullptr) {
                out.push_back(func());

                if (eat<Key>(until) == nullptr) {
                    expect<Key>(sep);
                } else {
                    break;
                }
            }

            return out;
        }

        template<typename T, typename F>
        T* decorators(F&& func) {
            bool found = false;

            vector<Decorator*> decorators;

            while (eat<Key>(Key::AT)) {
                found = true;

                if (eat<Key>(Key::LSQUARE)) {
                    do {
                        decorators.push_back(decorator());
                    } while (eat<Key>(Key::COMMA));
                    expect<Key>(Key::RSQUARE);
                } else {
                    decorators.push_back(decorator());
                }
            }

            if (!found) {
                return nullptr;
            }

            return new Decorated<T>(decorators, func());
        }

        Lexer* lexer;
        Token* ahead = nullptr;
    };
}
