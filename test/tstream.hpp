#pragma once

#include <stream.hpp>
#include "test.hpp"

using namespace cthulhu;

struct FileStream : StreamHandle {
    FileStream(const char* path, bool check = true): file(fopen(path, "r")) { 
        if (check) {
            ASSERT(file != nullptr);
        }
    }

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