#include <cthulhu.h>

using namespace cthulhu;
using namespace std;

struct Printer: Visitor {

};

int main() {
    try {
        init();

        Context ctx;
        //Printer print;

        auto source = Builder(R"(
            record a { field: void } 
            record b { field: void }
        )");

        auto unit = source.build(&ctx);

        unit->sema(&ctx);
        //unit.visit(&print);
    } catch (const runtime_error& err) {
        fmt::print("{}\n", err.what());
    }
}
