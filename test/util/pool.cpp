#include <pool.hpp>
#include "test.hpp"

using namespace cthulhu;

int main() {
    StringPool pool;

    auto* first = pool.intern("first");
    auto* second = pool.intern("second");
    auto* third = pool.intern("first");
    auto* fourth = pool.intern("first-second");

    ASSERT(first != second);
    ASSERT(second != third);
    ASSERT(first == third);
    ASSERT(first != fourth);
    ASSERT(third != fourth);
}
