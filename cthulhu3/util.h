#pragma once

#include <unordered_set>
#include <string>

using ident = const std::string*;

struct pool {
    ident intern(const std::string& string);
private:
    std::unordered_set<std::string> data;
};

struct stream {
    struct handle {
        virtual ~handle() { }
        virtual char next() = 0;
    };

    stream(handle* self);

    char next();
    char peek();

private:
    handle* self;
    char ahead;
};
