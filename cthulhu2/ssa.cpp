#include "ssa.h"

namespace ctu::ssa {
    // constant folding
    Operand opfold(State* state, Operand op) {
        if (op.imm) {
            return op;
        }

        auto step = state->steps[op.value];

        // this step produces side effects so cant be folded
        if (step->flags & Step::EFFECTS) {
            return op;
        }

        if (auto* val = dynamic_cast<Value*>(step); val) {
            return Operand(val->value, true);
        } else if (auto* func = dynamic_cast<Label*>(step); func) {
            return opfold(state, func->result);
        } else {
            return Operand(op.value);
        }
    }

    void Binary::fold(State* state) { 
        if (!foldable()) return;
        lhs = opfold(state, lhs);
        rhs = opfold(state, rhs);

        if (lhs.imm && rhs.imm) {
            switch (op) {
            case BinOp::ADD: 
                state->update(index, state->make<Value>(index, lhs.value + rhs.value)); 
                break;
            case BinOp::SUB: 
                state->update(index, state->make<Value>(index, lhs.value - rhs.value)); 
                break;
            case BinOp::DIV: 
                state->update(index, state->make<Value>(index, lhs.value / rhs.value));
                break;
            case BinOp::MUL: 
                state->update(index, state->make<Value>(index, lhs.value * rhs.value)); 
                break;
            case BinOp::REM: 
                state->update(index, state->make<Value>(index, lhs.value % rhs.value)); 
                break;
            default: 
                panic("unknown binop");
            }
        }
    }

    void Label::fold(State* state) { 
        if (!foldable()) return;
        result = opfold(state, result);
    }

    void Call::fold(State* state) { 
        if (!foldable()) return;
        func = opfold(state, func);
        if (func.imm) {
            state->update(index, state->make<Value>(index, func.value));
        }
    }

    void Select::fold(State* state) {
        if (!foldable()) return;

        cond = opfold(state, cond);
        yes = opfold(state, yes);
        no = opfold(state, no);
        if (cond.imm) {
            state->update(index, state->make<Value>(index, cond.value ? yes.value : no.value));
        }
    }

    // dead code removal
    void opreduce(State* state, Operand op) {
        if (!op.imm) {
            state->used[op.value]++;
        }
    }

    void Step::reduce(State* state) {
        if (flags & Step::PRESERVE) {
            state->used[index]++;
        }
    }

    void Binary::reduce(State* state) {
        opreduce(state, lhs);
        opreduce(state, rhs);
    }

    void Label::reduce(State* state) {
        Step::reduce(state);
        opreduce(state, result);
    }

    void Call::reduce(State* state) {
        Step::reduce(state);
        opreduce(state, func);
    }

    void Select::reduce(State* state) {
        Step::reduce(state);
        opreduce(state, cond);
        opreduce(state, yes);
        opreduce(state, no);
    }
}
