#include "test.hpp"

int main() {
    auto* lex = cthulhu::string(u8"int *int");
    auto parse = cthulhu::Parser(lex);

    auto* type = parse.type();
    ASSERT(type != nullptr);

    auto* qual = dynamic_cast<cthulhu::Qualified*>(type);
    ASSERT(qual != nullptr);

    ASSERT(qual->names.size() == 1);
    ASSERT(qual->names[0]->name == u8"int");



    auto* ptr = parse.type();

    printf("%s\n", ptr->repr().c_str());
}
