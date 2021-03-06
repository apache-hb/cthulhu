#pragma once

#include "test.hpp"
#include <stream.hpp>

using namespace cthulhu;

struct FileStream : StreamHandle {
#ifdef _WIN32
    FileStream(const char* path, bool check = true) { 
        errno_t err = fopen_s(&file, path, "r");
        if (check) {
            ASSERT(err == 0);
            ASSERT(file != nullptr);
        }
    }
#else
    FileStream(const char* path, bool check = true): file(fopen(path, "r")) { 
        if (check) {
            ASSERT(file != nullptr);
        }
    }
#endif

    ~FileStream() {
        fclose(file);
    }

    virtual c32 next() override {
        return fgetc(file);
    }

private:
    FILE* file;
};

struct StringStream : StreamHandle {
    StringStream(const utf8::string& data)
        : data(data)
        , offset(0)
    { }

    virtual c32 next() override {
        if (offset >= data.length()) {
            return END;
        } else {
            return data[offset++];
        }
    }

private:
    utf8::string data;
    size_t offset;
};