#pragma once

namespace ctu::ssa2 {
    struct Operand {
        // operand types
        enum Type : int {
            REG, /// this points to a register
            IMM, /// this is an immediate value
        };

        // create a new operand
        // default the type to REG to simplify 
        // emitting SSA
        Operand(size_t index, Type type = REG)
            : index(index)
            , type(type)
        { }

        bool imm() const { return type == IMM; }
        bool reg() const { return type == REG; }

        void imm(size_t value) {
            index = value;
            type = IMM;
        }

        void reg(size_t value) {
            index = value;
            type = REG;
        }

        // index has many meanings
        // if flags = REG then index is the name of a register
        // if flags = IMM then index is a scalar value
        size_t index;
        Type type;
    };

    struct Step {
        Step(size_t index = ~0ULL)
            : index(index)
        { }

        // this might point to a register that hasnt been assigned yet
        bool resolved() const { return index != ~0ULL; }

        // the register this step is assigned to
        // every step is assigned a register
        // no exceptions, even branches get a register
        size_t index;
    };

    // a value
    // reg = imm
    struct Value: Step {

    };

    // control flow label
    // reg = label
    struct Label: Step {

    };

    // function definition
    // reg = define label params
    struct Define: Step {

    };

    // call a function definition
    // reg = call op params
    struct Call: Step {

    };

    // control flow branch, can also be used as a jmp when cond is an immediate truthy value
    // reg = br cond label
    struct Branch: Step {
        Operand cond;
        Operand label;
    };
}
