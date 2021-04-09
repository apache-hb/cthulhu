#include <cthulhu.h>

using namespace cthulhu;
using namespace std;

int main() {
    try {
        init();

        auto ctx = Context("record a { field: void } record b { field: void }");

    } catch (const runtime_error& err) {
        fmt::print("{}\n", err.what());
    }
}
