#pragma once

#include "cthulhu.hpp"

#include <exception>

#define ASSERT(expr) if (!(expr)) { throw new std::runtime_error("ASSERT: " #expr "\n"); }

namespace cthulhu {
    struct StringStreamHandle : StreamHandle {
        StringStreamHandle(utf8::string source) 
            : source(source)
            , offset(0) 
        { }

        virtual char32_t next() override {
            return source.length() > offset ? source[offset++] : cthulhu::END;
        }

        utf8::string source;
        size_t offset;
    };

    struct FileStreamHandle : StreamHandle {
        FileStreamHandle(const char* path)
            : fd(fopen(path, "r"))
        { }

        virtual char32_t next() override {
            return fgetc(fd);
        }

        FILE* fd;
    };

    struct DebugLexer : Lexer {
        DebugLexer(StreamHandle* handle) 
            : Lexer(Stream(handle)) 
        { }

        template<typename T, typename... A>
        void expect(A&&... args) {
            Token* token = read();
            ASSERT(token != nullptr);

            auto* as = dynamic_cast<T*>(token);
            ASSERT(as != nullptr);

            ASSERT(*as == T(args...));
        }
    };

    DebugLexer* string(utf8::string code) {
        StringStreamHandle* handle = new StringStreamHandle(code);
        DebugLexer* lexer = new DebugLexer(handle);
        return lexer;
    }

    DebugLexer* open(const char* path) {
        FileStreamHandle* handle = new FileStreamHandle(path);
        DebugLexer* lexer = new DebugLexer(handle);
        return lexer;
    }
}
