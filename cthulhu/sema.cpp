#include "cthulhu.h"

using namespace ctu;

Type* Sentinel::resolve(Context* ctx) {
    for (auto type : ctx->types) {
        if (type->name == name) {
            return type;
        }
    }

    panic("unable to resolve `{}`", name);
}

struct Fixup: ctu::Visitor {
    Fixup(ctu::Context* ctx)
        : ctx(ctx)
    { }

    ctu::Context* ctx;

    virtual void visit(EmptyFunction* node) { 
        node->result = node->result->resolve(ctx);
        for (auto& each : node->params) {
            each.type = each.type->resolve(ctx);
        }
    }

    virtual void visit(LinearFunction* node) { 
        node->result = node->result->resolve(ctx);
        for (auto& each : node->params) {
            each.type = each.type->resolve(ctx);
        }
    }

    virtual void visit(BlockFunction* node) { 
        node->result = node->result->resolve(ctx);
        for (auto& each : node->params) {
            each.type = each.type->resolve(ctx);
        }
    }

    virtual void visit(Context* node) { 
        for (auto decl : node->globals) 
            decl->visit(this);
    }
};

namespace ctu {
    void fixup(Context* ctx) {
        Fixup fix = { ctx };
        ctx->visit(&fix);
    }
}
