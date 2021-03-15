#pragma once

#include <string>
#include <stddef.h>
#include <stdio.h>

struct StreamHandle {
    virtual ~StreamHandle() { }
    virtual char next() = 0;
};

struct FileStream : StreamHandle {
    FileStream(FILE* file) : file(file) { }

    virtual ~FileStream() override { }

    virtual char next() override {
        int i = fgetc(file);
        return i == -1 ? '\0' : (char)i;
    }

private:
    FILE* file;
};

struct TextStream : StreamHandle {
    TextStream(std::string text) : text(text), offset(0) { }

    virtual ~TextStream() override { }

    virtual char next() override { 
        return text.length() < offset ? 0 : text[offset++];
    }

private:
    std::string text;
    size_t offset;
};

struct Stream {
    Stream(StreamHandle* handle);

    char next();
    char peek();

    StreamHandle* handle;
    char ahead;
};
