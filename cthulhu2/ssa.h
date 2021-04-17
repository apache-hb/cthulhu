#pragma once

#include "cthulhu.h"

namespace ctu::ssa {
    struct Operand {
        enum Flags : int {
            RESOLVED = (1 << 0),
            IMM = (1 << 1)
        };

        Operand(size_t reg, int flags = 0)
            : value(reg)
            , flags(flags)
        { }

        std::string debug() const {
            return fmt::format(imm() ? "${}" : "%{}", value);
        }

        bool resolved() const { return flags & RESOLVED; }
        bool imm() const { return flags & IMM; }
        
        void resolve(size_t idx) { 
            value = idx;
            flags |= RESOLVED; 
        }

        void set(size_t val) { 
            value = val;
            flags |= IMM;
        }

        size_t value;
        int flags;
    };

    struct Step {
        virtual ~Step() = default;

        virtual void fold(struct State*) { }
        virtual void reduce(struct State* state);

        enum Flags : int {
            NONE = 0,
            EFFECTS = (1 << 0), // does this step produce obvserable side effects
            PRESERVE = (1 << 1) // should this step be ignored by reduction pass
        };

        virtual std::string debug() const { 
            if (flags == NONE) {
                return "";
            }

            std::vector<std::string> out;
            
            if (flags & EFFECTS) {
                out.push_back("effects");
            }

            if (flags & PRESERVE) {
                out.push_back("preserve");
            }

            return fmt::format("# {}", fmt::join(out, "+ "));
        }

        bool foldable() const { return !(flags & EFFECTS); }

        // where this step is in the ir 
        size_t index = ~0ULL;

        // flags this step has
        int flags = NONE;
    };

    using BinOp = ctu::Binary::Op;

    struct Binary: Step {
        virtual ~Binary() = default;

        virtual std::string debug() const override {
            return fmt::format("%{} = {} {} {} {}\n",
                index, ctu::Binary::binop(op),
                lhs.debug(), rhs.debug(),
                Step::debug()
            );
        }

        virtual void fold(State*) override;
        virtual void reduce(State*) override;

        Binary(BinOp op, size_t lhs, size_t rhs)
            : op(op)
            , lhs(lhs)
            , rhs(rhs)
        { }

        BinOp op;
        Operand lhs;
        Operand rhs;
    };

    struct Label: Step {
        virtual ~Label() = default;

        virtual std::string debug() const override {
            return fmt::format("%{} = label {} <- {} {}\n", 
                index, name, result.debug(),
                Step::debug()
            );
        }

        virtual void fold(State*) override;
        virtual void reduce(State*) override;

        Label(std::string name, size_t result)
            : name(name)
            , result(result)
        { }

        std::string name;
        Operand result;
    };

    struct Value: Step {
        virtual ~Value() = default;

        virtual std::string debug() const override {
            return fmt::format("%{} = ${} {}\n", index, value, Step::debug());
        }

        Value(size_t value)
            : value(value)
        { }

        size_t value;
    };

    struct Call: Step {
        virtual ~Call() = default;

        virtual std::string debug() const override {
            return fmt::format("%{} = call {} {}\n", index, func.debug(), Step::debug());
        } 

        virtual void fold(State*) override;
        virtual void reduce(State*) override;

        Call(size_t func)
            : func(func)
        { }

        Operand func;
    };

    struct Select: Step {
        virtual ~Select() = default;

        virtual std::string debug() const override {
            return fmt::format("%{} = select ({}) {}, {} {}\n", 
                index, 
                cond.debug(), yes.debug(), no.debug(),
                Step::debug()
            );
        }

        virtual void fold(State*) override;
        virtual void reduce(State*) override;

        Select(size_t cond, size_t yes, size_t no)
            : cond(cond)
            , yes(yes)
            , no(no)
        { }

        Operand cond;
        Operand yes;
        Operand no;
    };

    struct State {
        template<typename T, typename... A>
        T* make(size_t index, const A&... args) {
            T* out = new T(args...);
            out->index = index;
            return out;
        }

        template<typename T, typename... A>
        T* add(const A&... args) {
            T* out = new T(args...);
            out->index = steps.size();
            steps.push_back(out);
            return out;
        }

        template<typename T, typename... A>
        T* addv(const A&... args) {
            auto* out = add<T>(args...);
            out->flags |= Step::EFFECTS;
            return out;
        }

        template<typename T, typename... A>
        T* addp(const A&... args) {
            auto* out = add<T>(args...);
            out->flags |= Step::PRESERVE;
            return out;
        }

        // constant folding
        void fold() {
            dirty = false;
            apply([&](auto step) { 
                step->fold(this); 
            });
        }

        // dead code elimination
        void reduce() {
            dirty = false;
            used = {};
            apply([&](auto step) { step->reduce(this); });

            apply([&](auto step) { 
                if (used[step->index] > 0) return;

                update(step->index, nullptr);
            });
        }

        void update(size_t index, Step* step) {
            auto* temp = steps[index];
            steps[index] = step;
            delete temp;
            dirty = true;
        }

        template<typename F>
        void apply(F&& func) {
            for (auto step : steps) {
                if (!step) continue;
                func(step);
            }
        }

        bool dirty = false;
        std::map<size_t, size_t> used;
        std::vector<Step*> steps;
    };
}
