#include <cthulhu.h>

using namespace cthulhu;
using namespace std;

struct X86: Context {
    X86() : Context() {

        // these are all the builtin types
        builtins = {
            new ast::VoidType(),
            new ast::BoolType(),

            new ast::ScalarType("char", 1, true),
            new ast::ScalarType("short", 2, true),
            new ast::ScalarType("int", 4, true),
            new ast::ScalarType("long", 8, true),

            new ast::ScalarType("uchar", 1, false),
            new ast::ScalarType("ushort", 2, false),
            new ast::ScalarType("uint", 4, false),
            new ast::ScalarType("ulong", 8, false),
        };
    }
};

int main() {
    try {
        init();

        X86 ctx;

        auto source = Builder(R"(
            record a { field: *void, $: int, $: int } 
            #record b { field: [int:4 + 4] }
        )");

        source.build(&ctx);
    } catch (const runtime_error& err) {
        fmt::print("{}\n", err.what());
    }
}
