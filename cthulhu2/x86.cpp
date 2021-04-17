#include "x86.h"

#if 0
// keep this in line with the prefix enum
// should be easy enough, i doubt intel is going to 
// add more prefixes to v8086 anytime soon
namespace consts::prefixes {
    constexpr uint8_t LOCK = 0xF0;
    constexpr uint8_t REPNE = 0xF2;
    constexpr uint8_t REP = 0xF3;
    constexpr uint8_t BND = 0xF2;
    constexpr uint8_t CS = 0x2E;
    constexpr uint8_t SS = 0x36;
    constexpr uint8_t DS = 0x3E;
    constexpr uint8_t ES = 0x26;
    constexpr uint8_t FS = 0x64;
    constexpr uint8_t GS = 0x65;
    constexpr uint8_t BNT = 0x2E;
    constexpr uint8_t BT = 0x3E;
    constexpr uint8_t SIZE = 0x66;
    constexpr uint8_t ADDR = 0x67;

    constexpr uint8_t ALL[] = {
        LOCK, REPNE, REP, BND,
        CS, SS, DS, ES, FS, GS, BNT, BT,
        SIZE,
        ADDR
    };
}

namespace ctu::x86 {
    namespace prefix {
        size_t encode(uint8_t* out, int prefixes) {
            if (prefixes == NONE) {
                return 0;
            }

            size_t index = 0;
            
            for (int i = 0; i < sizeof(ALL); i++) {
                if (prefixes & (1 << i)) {
                    out[index++] = consts::prefixes::ALL[i];
                }
            }

            return index;
        }
    }
}
#endif