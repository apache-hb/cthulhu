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

        void Fields::sema(Context* ctx, bool record) {
            bool bounded = true;
            for (auto [name, type] : *this) {
                if (type->unit(ctx)) {
                    panic("field `{}` is void", name);
                }
                type->sema(ctx);

                if (record) {
                    if (type->unsized()) {
                        bounded = false;
                        continue;
                    }

                    if (!bounded) {
                        panic("field `{}` with unbounded size must be last element of record", name);
                    }
                }
            }
        }

        void Cases::add(std::string name, Fields entry) {
            if (name == "$") {
                panic("discarded variant option");
            }

            for (auto [id, _] : *this) {
                if (id == name) {
                    panic("duplicate variant option `{}`", id);
                }
            }

            push_back({ name, entry });
        }

        void Cases::sema(Context* ctx) {
            for (auto [name, fields] : *this) {
                fields.sema(ctx);
            }
        }

        void Types::sema(Context* ctx) {
            for (auto type : *this) {
                type->sema(ctx);
            }
        }

        void PointerType::sema(Context* ctx) {
            auto base = root(ctx);
            ctx->enter(base, false, true, [&] { });
        }

        void ArrayType::sema(Context* ctx) {
            auto base = root(ctx);
            ctx->enter(base, true, true, [&] {
                type->sema(ctx);
            });

            if (size) {
                size->sema(ctx);
                if (!size->constant()) {
                    panic("size of array must be a constant expression");
                }
                if (!size->type(ctx)->scalar()) {
                    panic("size of array must be a scalar value");
                }
            }
        }

        void ClosureType::sema(Context* ctx) {
            ctx->enter(this, false, true, [&] {
                result->sema(ctx);
                args.sema(ctx);
            });
        }

        void RecordType::sema(Context* ctx) {
            ctx->enter(this, true, false, [&] {
                fields.sema(ctx, true);
            });
        }

        void AliasType::sema(Context* ctx) {
            ctx->enter(this, false, false, [&] {
                type->sema(ctx);
            });
        }

        void SumType::sema(Context* ctx) {
            if (parent) {
                if (parent->unit(ctx)) {
                    panic("variant `{}` may not extend unit type", name);
                }

                ctx->enter(this, false, false, [&] {
                    parent->sema(ctx);
                });
            }

            ctx->enter(this, true, false, [&] {
                cases.sema(ctx);
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

        bool SentinelType::unit(Context* ctx) const {
            return ctx->get(name)->unit(ctx);
        }

        void Binary::sema(Context* ctx) {
            if (lhs->type(ctx) != rhs->type(ctx)) {
                panic("binary operand types did not match");
            }
        }

        Type* Binary::type(Context* ctx) {
            ASSERT(lhs->type(ctx) == rhs->type(ctx));
            return lhs->type(ctx);
        }

        void IntLiteral::sema(Context* ctx) {
            if (!type(ctx)->resolved()) {
                panic("unknown int suffix `{}`", suffix);
            }
        }

        Type* IntLiteral::type(Context* ctx) {
            return ctx->get(suffix.empty() ? "int" : suffix);
        }

        Type* BoolLiteral::type(Context* ctx) {
            return ctx->get("bool");
        }

        void Name::sema(Context*) {
            // TODO:
            panic("TODO: name sema");
        }

        bool Name::constant() const {
            panic("TODO: name constant");
        }

        Type* Name::type(Context*) {
            // TODO:
            panic("TODO: resolve name");
        }

        void Function::sema(Context* ctx) {
            result->sema(ctx);
            bool defs = false;
            for (auto [id, type, init] : params) {
                type->sema(ctx);
                if (init) {
                    init->sema(ctx);
                    defs = true;
                } else {
                    if (defs) {
                        panic("parameter `{}` was not default initialized after a default initialized parameter", id);
                    }
                }
                if (type->unit(ctx)) {
                    panic("parameter `{}` for function `{}` has a unit type", id, name);
                }
            }
        }

        void SimpleFunction::sema(Context* ctx) {
            Function::sema(ctx);
        }

        void ComplexFunction::sema(Context* ctx) {
            Function::sema(ctx);
        }
    }

    void Context::add(ast::Function* func) {
        if (func->name == "$") {
            panic("function names may not be discarded");
        }

        for (auto& fun : funcs) {
            if (fun->name == func->name) {
                panic("multiple definitions of function `{}`", func->name);
            }
        }

        funcs.push_back(func);
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
