#include "cthulhu.hpp"

struct FileStreamHandle : cthulhu::StreamHandle {
    FileStreamHandle(const char* path)
        : fd(fopen(path, "r"))
    { }

    virtual char32_t next() override {
        return fgetc(fd);
    }

    FILE* fd;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("ast <in> <out>\n");
        exit(1);
    }
    
    auto file = FileStreamHandle(argv[1]);
    auto lexer = cthulhu::Lexer(&file);
    auto parser = cthulhu::Parser(&lexer);
    auto* unit = parser.unit();

    cthulhu::Printer printer;
    unit->visit(&printer);

    auto out = fopen(argv[2], "w");
    fwrite(printer.buffer.c_str(), printer.buffer.size(), 1, out);
    fclose(out);
    printf("dumped ast to %s\n", argv[2]);
}
