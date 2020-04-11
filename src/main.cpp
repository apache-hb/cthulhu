#include "lib/parse.h"

#include <algorithm>

int main(int argc, const char** argv)
{
    std::string test = R"(
        import x::y => jeff
    )";

    ctu::sstream in(&test);
    ctu::Lexer lex(&in);
    ctu::Parser parse(&lex);

    auto prog = parse.parse();
    
    for(auto* imp : prog->imports)
    {
        printf("import ");
        
        for(int i = 0; i < imp->path->path.size(); i++)
        {
            if(i != 0)
                printf(".");
            printf("%s", imp->path->path[i]->name.c_str());
        }
        
        if(imp->alias)
            printf(" as %s;\n", imp->alias->name.c_str());
    }
}