#include "ast.h"

#include "cthulhu.h"

namespace ctu {
    Type* Record::resolve(Context* ctx) {
        for (auto& field : fields) {
            field.type = field.type->resolve(ctx);
        }

        return this;
    }

    Type* Sentinel::resolve(Context* ctx) {
        return ctx->get(name)->resolve(ctx);
    }

    Type* Pointer::resolve(Context* ctx) {
        type = type->resolve(ctx);
        
        return this;
    }

    Type* Function::resolve(Context* ctx) {
        result = result->resolve(ctx);
        for (auto& param : params) {
            param.type = param.type->resolve(ctx);
        }
        
        return this;
    }

    void Scope::define(Symbol* symbol) {
        if (get(symbol->name) != nullptr) {
            ctu::panic("`{}` already defined", symbol->name);
        }

        symbols.push_back(symbol);
    }

    Symbol* Scope::get(std::string name) {
        for (auto symbol : symbols) {
            if (symbol->name == name) {
                return symbol;
            }
        }

        return nullptr;
    }

    void Context::define(Symbol* symbol) {
        for (auto& scope : scopes) {
            if (scope.get(symbol->name) != nullptr) {
                ctu::panic("`{}` already defined", symbol->name);
            }
        }

        scopes.back().define(symbol);
    }

    Symbol* Context::get(std::string name) {
        for (auto& scope : scopes) {
            if (auto symbol = scope.get(name); symbol != nullptr) {
                return symbol;
            }
        }

        ctu::panic("`{}` not defined", name);
    }

    void Context::push() {
        scopes.emplace_back();
    }

    void Context::pop() {
        scopes.pop_back();
    }
}
