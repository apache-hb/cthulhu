#pragma once

#include "cthulhu.h"

namespace ctu::tac {
    struct Operand {
        enum Type {
            IMM, // immediate value
            REG, // register value
            STUB // unresolved
        };

        std::string debug() const {
            switch (type) {
            case IMM: return fmt::format("${}", index);
            case REG: return fmt::format("%{}", index);
            case STUB: return "uninit";
            default: panic("unknown operand type");
            }
        }

        constexpr Operand(size_t index, Type type = REG)
            : index(index)
            , type(type)
        { }

        void resolve(size_t in) {
            type = REG;
            index = in;
        }

        size_t index;
        Type type;
    };

    constexpr Operand stub() {
        return Operand(0, Operand::STUB);
    }

    constexpr Operand imm(size_t v) {
        return Operand(v, Operand::IMM);
    }

    constexpr Operand reg(size_t r) {
        return Operand(r, Operand::REG);
    }

    struct Step {
        virtual ~Step() = default;
        
        virtual std::string debug() const = 0;

        size_t index;
    };

    struct Value: Step {
        virtual ~Value() = default;

        virtual std::string debug() const override {
            return fmt::format("    %{} = {}", index, value.debug());
        }

        Value(Operand value)
            : value(value)
        { }

        Operand value;
    };

    struct Binary: Step {
        virtual ~Binary() = default;

        virtual std::string debug() const override {
            return fmt::format("    %{} = {} {} {}",
                index, 
                ctu::Binary::binop(op), 
                lhs.debug(),
                rhs.debug()
            );
        }

        Binary(Operand lhs, Operand rhs, ctu::Binary::Op op)
            : lhs(lhs)
            , rhs(rhs)
            , op(op)
        { }

        Operand lhs;
        Operand rhs;
        ctu::Binary::Op op;
    };

    struct Unary: Step {
        virtual ~Unary() = default;

        virtual std::string debug() const override {
            return fmt::format("    %{} = {} {}", 
                index,
                ctu::Unary::unop(op), 
                expr.debug()
            );
        }

        Unary(Operand expr, ctu::Unary::Op op)
            : expr(expr)
            , op(op)
        { }

        Operand expr;
        ctu::Unary::Op op;
    };

    struct Label: Step {
        virtual ~Label() = default;

        virtual std::string debug() const override {
            return fmt::format("{}:", index);
        }
    };

    struct Branch: Step {
        virtual ~Branch() = default;

        virtual std::string debug() const override {
            return fmt::format("    if {} goto {}", cond.debug(), label.debug());
        }

        Branch(Operand cond, Operand label)
            : cond(cond)
            , label(label)
        { }

        Operand cond;
        Operand label;
    };

    struct Call: Step {
        virtual ~Call() = default;

        virtual std::string debug() const override {
            std::vector<std::string> strs;
            for (auto op : args) {
                strs.push_back(op.debug());
            }

            return fmt::format("    %{} = call {} ({})", 
                index, 
                func.debug(),
                fmt::join(strs, ", ")
            );
        }

        Call(Operand func, std::vector<Operand> args)
            : func(func)
            , args(args)
        { }

        Operand func;
        std::vector<Operand> args;
    };

    struct Return: Step {
        virtual ~Return() = default;

        virtual std::string debug() const override {
            return fmt::format("    ret {}", val.debug());
        }

        Return(Operand val)
            : val(val)
        { }

        Operand val;
    };

    struct Jump: Step {
        virtual ~Jump() = default;

        virtual std::string debug() const override {
            return fmt::format("    goto {}", label.debug());
        }

        Jump(Operand label)
            : label(label)
        { }

        Operand label;
    };

    struct Unit {
        template<typename T, typename... A>
        T* step(A&&... args) {
            T* out = new T(args...);
            out->index = steps.size();
            steps.push_back(out);
            return out;
        }

        std::vector<Step*> steps;
    };
}
