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
        auto it = ctx->get(name);
        
        if (it == nullptr) {
            ctu::panic("unresolved type `{}`", name);
        }

        return it->resolve(ctx);
    }

    std::string Sentinel::debug() const {
        return fmt::format("(sentinel {})", name);
    }

    Type* Builtin::resolve(Context*) {
        return this;
    }

    std::string Builtin::debug() const {
        return fmt::format("(builtin {})", name);
    }

    Type* Pointer::resolve(Context* ctx) {
        type = type->resolve(ctx);
        
        return this;
    }

    std::string Pointer::debug() const {
        return fmt::format("(ptr {})", type->debug());
    }

    Type* Function::resolve(Context* ctx) {
        result = result->resolve(ctx);
        for (auto& param : params) {
            param.type = param.type->resolve(ctx);
        }
        
        return this;
    }

    std::string Function::debug() const {
        return fmt::format("(def {} {})", name, result->debug());
    }

    std::string SimpleFunction::debug() const {
        return fmt::format("(def {} {} {})", name, result->debug(), expr->debug());
    }

    std::string IntLiteral::debug() const {
        return fmt::format("(int {})", value);
    }

    std::string BoolLiteral::debug() const {
        return fmt::format("(bool {})", value);
    }


    std::string Binary::debug() const {
        return fmt::format("({} {} {})", str(op), lhs->debug(), rhs->debug());
    }

    std::string Unary::debug() const {
        return fmt::format("({} {})", str(op), expr->debug());
    }

    std::string Call::debug() const {
        return fmt::format("(call {})", body->debug());
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

        return nullptr;
    }

    void Context::push() {
        scopes.emplace_back();
    }

    void Context::pop() {
        scopes.pop_back();
    }
}
