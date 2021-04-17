#include <cthulhu.h>
#include <ssa.h>
#include <fstream>

using namespace ctu;

struct SSA: Visitor {
    virtual ~SSA() = default;

    SSA(ssa::State* state, ctu::Context* ctx)
        : state(state)
        , ctx(ctx)
    { 
        for (auto decl : ctx->globals) {
            decl->visit(this);
        }
    }

    virtual void visit(Literal* expr) override {
        expr->index = state->add<ssa::Value>(expr->value)->index;
    }

    virtual void visit(Binary* expr) override { 
        expr->lhs->visit(this);
        expr->rhs->visit(this);
        expr->index = state->add<ssa::Binary>(expr->op, expr->lhs->index, expr->rhs->index)->index;
    }

    virtual void visit(Call* expr) override { 
        expr->body->visit(this);
        expr->index = state->add<ssa::Call>(expr->body->index)->index;
    }

    virtual void visit(Name* expr) override { 
        expr->index = ctx->find(expr->name);
    }

    virtual void visit(Ternary* expr) override { 
        expr->cond->visit(this);
        expr->yes->visit(this);
        expr->no->visit(this);
        
        expr->index = state->add<ssa::Select>(
            expr->cond->index,
            expr->yes->index, 
            expr->no->index
        )->index;
    }

    virtual void visit(Function* node) override {
        iter = 0;
        node->expr->visit(this);
        node->index = state->addp<ssa::Label>(node->name, node->expr->index)->index;
    }

    size_t iter = 0;
    ssa::State* state;
    ctu::Context* ctx;
    std::map<size_t, ssa::Step*> labels;
};



int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << ": no source files provided" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    if (std::ifstream in(path); !in.fail()) {
        auto text = std::string(std::istreambuf_iterator<char>{in}, {});
        
        try {
            ctu::init();

            ctu::Context ctx = ctu::parse(text);
            ssa::State state;

            SSA visit(&state, &ctx);

            state.apply([](auto step) {
                fmt::print(step->debug());
            });

            int iter = 0;

            do { 
                state.fold();
                state.reduce();
                iter++;
            } while (state.dirty);

            fmt::print("-- reduced `{}` times --\n", iter);

            state.apply([](auto step) {
                fmt::print(step->debug());
            });

        } catch (const std::exception& error) {
            std::cerr << error.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "failed to open: " << path << std::endl;
        return 1;
    }
}
