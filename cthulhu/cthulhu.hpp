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

        Unit* parseUnit();
        Node* parseInclude();
        Id* parseIdent();
        
        Decl* parseDecl();
        TemplateItems* parseTemplate();
        TemplateItem* parseTemplateItem();
        BaseDecl* parseBaseDecl();

        Alias* parseAlias();

        Type* parseType();
        BaseType* parseBaseType();
        Qualified* parseQualified();
        Pointer* parsePointer();
        Array* parseArray();
        Closure* parseClosure();
        Name* parseName();

        Expr* parseExpr();

        Decorators* parseDecorators();
        Decorator* parseDecorator();

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

        // parse one or more `T` seperated by `sep`
        template<typename T, typename F>
        List<T>* parseSome(Key::Word sep, F&& func) {
            auto* list = new List<T>();

            do list->add(func()); while (eat<Key>(sep));

            return list;
        }

        // parse none or more `T`
        template<typename T, typename F>
        List<T>* parseMany(F&& func) {
            auto* list = new List<T>();

            while (true) {
                if (auto* node = func(); !node) {
                    break;
                } else {
                    list->add(node);
                }
            }

            return list;
        }

        // parse none or more `T` seperated by `sep`
        template<typename T, typename F>
        List<T>* parseMany(Key::Word sep, F&& func) {
            auto* list = new List<T>();

            while (true) {
                if (auto* node = func(); !node) {
                    break;
                } else {
                    list->add(node);
                }
                expect<Key>(sep);
            }

            return list;
        }

        template<typename T, typename F>
        Arguments<T>* parseArguments(Key::Word start, Key::Word end, F&& func) {
            if (!eat<Key>(start)) {
                return nullptr;
            }

            auto* args = new Arguments<T>();

            do {
                Id* name;
                if (eat<Key>(Key::DOT)) {
                    name = parseIdent();
                    expect<Key>(Key::ASSIGN);
                } else {
                    name = nullptr;
                }

                T* arg = func();

                args->add(new Argument(name, arg));
            } while (eat<Key>(Key::COMMA));

            expect<Key>(end);

            return args;
        }

#if 0
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
#endif

        Lexer* lexer;
        Token* ahead = nullptr;
    };
}
