#pragma once

#include <peglib.h>
#include <fmt/core.h>

namespace ctu {
    template<typename T = std::runtime_error, typename... A>
    [[noreturn]] void panic(const char* fmt, const A&... args) {
        throw T(fmt::format(fmt, args...));
    }

    struct Context;

    struct Symbol {
        virtual ~Symbol() = default;

        Symbol(std::string name): name(name) { }

        virtual bool resolved() {
            return true;
        }

        std::string name;
    };

    struct Sentinel: Symbol {
        virtual ~Sentinel() = default;

        virtual bool resolved() override {
            return false;
        }

        Sentinel(std::string name): Symbol(name) { }
    };

    struct Decl: Symbol {
        virtual ~Decl() = default;

        Decl(std::string name): Symbol(name) { }
    };

    struct Type {
        virtual ~Type() = default;
    };

    struct NamedType: Type, Symbol {
        virtual ~NamedType() = default;

        NamedType(std::string name): Type(), Symbol(name) { }
    };

    struct Builtin: NamedType {
        virtual ~Builtin() = default;

        Builtin(std::string name): NamedType(name) { }
    };

    template<typename T>
    struct Symbols: std::vector<Symbol*> {
        using Super = std::vector<Symbol*>;

        Symbols(std::initializer_list<Symbol*> init)
            : Super(init)
        { }

        /**
        * add a symbol to this list of symbols.
        * if a sentinel is found replace it.
        * if the symbol was already defined then throw an exception.
        * multiple symbols called `$` may be added if `discard` is true
        * @param it: the symbol to add
        * @param discard: allow symbols called `$` to be added
        */
        void add(T* it, bool discard = false) {
            if (it->name == "$") {
                if (discard) {
                    push_back(it);
                    return;
                } else {
                    panic("discarding symbols is invalid in this context");
                }
            }

            // make sure this name isnt duplicated
            for (Symbol*& symbol : *this) {
                if (symbol->name == it->name) {
                    panic("redefinition of `{}` with `{}`", symbol->name, it->name);
                }
            }

            // if we didnt find the symbol then add it
            push_back(it);
        }

        // find a symbol by name
        // if the symbol isnt found then emplace a sentinel
        // and return that to be resolved later
        T* find(const std::string& name) {
            for (Symbol* symbol : *this) {
                if (symbol->name == name) {
                    return symbol;
                }
            }

            T* sentinel = new Sentinel(name);
            push_back(sentinel);
            return sentinel;
        }
    };

    struct Context {
        // all named declarations
        Symbols<Decl> decls = {
            new Builtin("bool"),
            new Builtin("void"),
            new Builtin("char"),
            new Builtin("short"),
            new Builtin("int"),
            new Builtin("long"),
            new Builtin("ssize")
        };
    };

    void init();
    Context parse(std::string source);
}