#include "cthulhu.h"

namespace cthulhu {
    namespace ast {
        void Fields::add(std::string name, Type* type) {
            if (name != "$") {
                for (auto [id, _] : *this) {
                    if (id == name) {
                        panic("duplicate field `{}` in fields", name);
                    }
                }
            }

            push_back({ name, type });
        }

        void Fields::sema(Context* ctx) {
            for (auto [name, type] : *this) {
                if (type->unit()) {
                    panic("field `{}` is void", name);
                }
                type->sema(ctx);
            }
        }

        void PointerType::sema(Context* ctx) {
            auto base = root(ctx);
            ctx->enter(base, false, true, [&] { });
        }

        void ArrayType::sema(Context* ctx) {
            auto base = root(ctx);
            ctx->enter(base, false, true, [&] {
                type->sema(ctx);
            });

            if (size) {
                if (!size->constant()) {
                    panic("size of array must be a constant expression");
                }
                // TODO: get type of expression
                /*if (!size->constant()) {
                    panic("size of array must be a constant expression");
                } */
            }
        }

        void RecordType::sema(Context* ctx) {
            ctx->enter(this, true, false, [&] {
                fields.sema(ctx);
            });
        }

        void AliasType::sema(Context* ctx) {
            ctx->enter(this, false, false, [&] {
                type->sema(ctx);
            });
        }

        void SentinelType::sema(Context* ctx) {
            auto self = ctx->get(name);

            if (self->resolved()) {
                self->sema(ctx);
            } else {
                panic("unresolved type `{}`", name);
            }
        }
    }

    void Context::add(ast::NamedType* type) {
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

    ast::NamedType* Context::get(std::string name) {
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

        auto sentinel = new ast::SentinelType(name);
        types.push_back(sentinel);
        return sentinel;
    }
}
