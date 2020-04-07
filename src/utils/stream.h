#pragma once

#include <stdint.h>

#include <string>

namespace ctu
{
    struct stream
    {
        virtual uint8_t next() = 0;
        virtual uint8_t peek() const = 0;

        virtual std::string range(uint32_t start, uint32_t len) = 0;
    };

    struct fstream : stream
    {
        fstream(FILE* f)
            : file(f)
        {}

        virtual uint8_t next() override 
        {
            return fgetc(file);
        }
        
        virtual uint8_t peek() const override 
        {
            auto c = fgetc(file);
            ungetc(c, file);
            return c;
        }

        virtual std::string range(uint32_t start, uint32_t len) override 
        {
            auto pos = ftell(file);
            fseek(file, start, SEEK_SET);

            char* chars = (char*)alloca(len);
            fread(chars, 1, len, file);

            fseek(file, pos, SEEK_SET);

            return std::string(chars, len);
        }

    private:
        FILE* file;
    };

    struct sstream : stream
    {
        sstream(std::string* s)
            : str(s)
            , idx(0)
        {}

        virtual uint8_t next() override 
        {
            return str->at(idx++);
        }
        
        virtual uint8_t peek() const override 
        {
            return str->at(idx);
        }

        virtual std::string range(uint32_t start, uint32_t len) override 
        {
            return std::string(&str->at(start), len);
        }
    private:
        std::string* str;
        int idx;
    };
}