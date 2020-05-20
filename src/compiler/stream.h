#include <stdint.h>
#include <string>
#include <memory>

struct FilePos {
    uint64_t dist = 0;
    uint64_t col = 0;
    uint64_t line = 0;
};

struct StreamHandle {
    virtual ~StreamHandle() {}
    virtual char get() = 0;
};

struct StringStream : StreamHandle {
    StringStream(std::string buf)
        : buffer(buf)
    { }

    virtual char get() override {
        return cursor < buffer.size() ? buffer[cursor++] : '\0';
    }

    uint64_t cursor = 0;
    std::string buffer;
};

struct FileStream : StreamHandle {
    FileStream(const char* path)
        : file(fopen(path, "rt"))
    { }

    virtual char get() override {
        return fgetc(file);
    }

    FILE* file;
};

struct Stream {
    Stream(StreamHandle* stream)
        : handle(stream)
    { 
        ahead = stream->get();
    }

    virtual ~Stream() {}

    char next() {
        char c = ahead;
        ahead = handle->get();

        if(c == '\n') {
            here.line++;
            here.col = 0;
        } else {
            here.col++;
        }

        here.dist++;

        return c;
    }
    
    char peek() {
        return ahead;
    }

    bool consume(char c) {
        if(peek() == c) {
            next();
            return true;
        }
        return false;
    }

    std::shared_ptr<StreamHandle> handle;

    FilePos here;
    char ahead;
};
