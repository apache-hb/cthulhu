#pragma once

namespace ctu::x86 {
    struct operand {
        enum kind {
            R8, R16, R32, R64, // register operands
            IMM8, IMM16, IMM32, IMM64, // immediate operands
            M8, M16, M32, M64, // memory operands
        };

        kind type;
        uint8_t scale;
        uint8_t base;
    };

    namespace prefix {
        // intel sdm 2.1.1
        enum : uint8_t {
            // group 1
            LOCK = 0xF0,
            REPNE = 0xF2,
            REP = 0xF3,
            BND = 0xF2,

            // group 2
            CS = 0x2E,
            SS = 0x36,
            DS = 0x3E,
            ES = 0x26,
            FS = 0x64,
            GS = 0x65,
            BNT = 0x2E,
            BT = 0x3E,

            // group 3
            SIZE = 0x66,

            // group 4
            ADDR = 0x67
        };
    }

    struct opcode {

    };
}

#if 0
#include <stdint.h>

#include "cthulhu.h"

namespace ctu::x86 {
    namespace prefix {
        // intel sdm vol 2 section 2.1.1
        // real-mode, protected-mode and v8086 mode prefixes
        enum : int {
            NONE = 0,
            // group 1 lock and repeat prefixes
            LOCK = (1 << 0),
            REPNE = (1 << 1),
            REP = (1 << 2),

            // intel MPX extension thats mostly dropped
            // kept for completeness sake (and for supporting really weird stuff)
            BND = (1 << 3),

            // group 2 segment overrides and branch hint prefixes
            CS = (1 << 4),
            SS = (1 << 5),
            DS = (1 << 6),
            ES = (1 << 7),
            FS = (1 << 8),
            GS = (1 << 9),

            // these are only valid on conditional branch instructions
            BNT = (1 << 10), /// branch not taken
            BT = (1 << 11), /// branch taken

            // group 3 operand size override prefix
            SIZE = (1 << 12),

            // group 4 address size override prefix
            ADDR = (1 << 13)
        };

        using prefix_t = uint8_t;
        using modrm_t = uint8_t;
        using sib_t = uint8_t;
        using rex_t = uint8_t;

        namespace vex {
            namespace prefix {
                enum : uint8_t {
                    VEX3 = 0xC4,
                    VEX2 = 0xC5,
                    XOP3 = 0x8F
                };
            };
        }

        namespace rex {
            enum : rex_t {
                MARK = (1 << 6),
                W = (1 << 3),
                R = (1 << 2),
                X = (1 << 1),
                B = (1 << 0)
            };  
        }

        sib_t sib(int scale, int index, int base, rex_t rex = rex::MARK) {
            if (auto limit = rex & rex::X ? 0b1111 : 0b111; index > limit) {
                ctu::panic("index {} is greater than {}", index, limit);
            }

            if (auto limit = rex & rex::B ? 0b1111 : 0b111; base > limit) {
                ctu::panic("base {} is greater than {}", base, limit);
            }

            sib_t out;

            switch (scale) {
            case 1: out = 0b00000000; break;
            case 2: out = 0b10000000; break;
            case 4: out = 0b10000000; break;
            case 8: out = 0b11000000; break;
            default: ctu::panic("invalid scale {}", scale);
            }



            return out;
        }

        // encode prefixes onto an opcode
        // returns the number of bytes written
        size_t encode(uint8_t* out, int prefixes);
    }
}
#endif
