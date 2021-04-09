#include "cthulhu.h"

namespace cthulhu {
    void ast::Fields::add(std::string name, std::shared_ptr<ast::Type> type) {
        if (name != "$") {
            for (auto [id, _] : *this) {
                if (id == name) {
                    panic("duplicate field `{}` in fields", name);
                }
            }
        }

        push_back({ name, type });
    }

    void ast::ArrayType::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::ArrayType>());
    }

    void ast::RecordType::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::RecordType>());
    }

    void ast::PointerType::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::PointerType>());
    }

    void ast::ScalarType::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::ScalarType>());
    }

    void ast::BoolType::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::BoolType>());
    }

    void ast::VoidType::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::VoidType>());
    }

    void ast::IntLiteral::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::IntLiteral>());
    }

    void ast::Binary::visit(const std::shared_ptr<Visitor> visitor) const {
        visitor->visit(as<ast::Binary>());
    }

    std::shared_ptr<ast::Type> ast::IntLiteral::type(Context* ctx) const {
        /* TODO: support stuff like `u` and `ul` */
        return ctx->get(suffix.empty() ? "int" : suffix);
    }

    std::shared_ptr<ast::Type> ast::Binary::type(Context* ctx) const {
        /* TODO: assert that both sides have the same type */
        return lhs->type(ctx);
    }

    void Context::add(std::shared_ptr<ast::NamedType> type) {
        if (type->name == "$") {
            panic("type names may not be discarded");
        }

        for (auto builtin : builtins) {
            if (builtin->name == type->name) {
                panic("redefinition of basic type `{}`", type->name);
            }
        }

        /* the & on this auto& is important */
        for (auto& def : types) {
            if (def->name == type->name) {
                /* if a sentinel is found then resolve it */
                if (def->resolved()) {
                    panic("multiple definitions of type `{}`", type->name);
                } else {
                    def = type;
                    return;
                }
            }
        }

        types.push_back(type);
    }

    std::shared_ptr<ast::NamedType> Context::get(std::string name) {
        for (auto builtin : builtins) {
            if (builtin->name == name) {
                return builtin;
            }
        }

        for (auto def : types) {
            if (def->name == name) {
                return def;
            }
        }

        auto sentinel = std::make_shared<ast::SentinelType>(name);
        types.push_back(sentinel);
        return sentinel;
    }
}
