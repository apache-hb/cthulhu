#include "AST/Types.h"

int main(int argc, char** argv)
{
    auto* t1 = new AST::BuiltinType(AST::Builtin::F64);
    auto* t2 = new AST::BuiltinType(AST::Builtin::U8);
    auto* t3 = new AST::BuiltinType(AST::Builtin::I32);

    auto* un = new AST::UnionType({
        { "a", t1 },
        { "b", t2 },
        { "c", t3 }
    });
}