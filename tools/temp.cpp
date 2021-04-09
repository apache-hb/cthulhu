#include <cthulhu.h>

using namespace cthulhu;
using namespace std;

struct Printer: Visitor {

};

struct X86: Context {
    X86() : Context() {

        // these are all the builtin types
        builtins = {
            std::make_shared<ast::VoidType>(),
            std::make_shared<ast::BoolType>(),

            std::make_shared<ast::ScalarType>("char", 1, true),
            std::make_shared<ast::ScalarType>("short", 2, true),
            std::make_shared<ast::ScalarType>("int", 4, true),
            std::make_shared<ast::ScalarType>("long", 8, true),

            std::make_shared<ast::ScalarType>("uchar", 1, false),
            std::make_shared<ast::ScalarType>("ushort", 2, false),
            std::make_shared<ast::ScalarType>("uint", 4, false),
            std::make_shared<ast::ScalarType>("ulong", 8, false),
        };
    }
};

int main() {
    try {
        init();

        X86 ctx;

        auto source = Builder(R"(
            record a { field: *void, $: int, $: int } 
            record b { field: [int:4 + 4char] }
        )");

        source.build(&ctx);

        ctx.dbg();

        //unit.visit(&print);
    } catch (const runtime_error& err) {
        fmt::print("{}\n", err.what());
    }
}
