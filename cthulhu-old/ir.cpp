#include "ir.hpp"

namespace cthulhu::ir {
    Unit validate(cthulhu::Unit* root, Unit(*include)(vector<utf8::string>)) {
        (void)root;
        (void)include;
        return {};
    }
}
