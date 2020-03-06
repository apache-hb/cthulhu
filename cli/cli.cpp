#include <ctulang/ctulang.h>

#include <sstream>

int main(int argc, const char** argv)
{
    std::stringstream ss;

    ss << "name jeff" << std::endl << "name jeff 2" << std::endl;
    ctu::error({
        &ss,
        "test",

        10,
        1,
        0,
        4,

        "yes",
        'T',
        0
    });
}