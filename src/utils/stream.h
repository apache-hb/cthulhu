#pragma once

#include <stdint.h>

#include <string>

namespace ctu
{
    struct stream
    {
        virtual ~stream() {}
        virtual uint8_t next() = 0;
        virtual uint8_t peek() const = 0;

        virtual std::string range(uint32_t start, uint32_t len) = 0;
    };

    struct fstream : stream
    {
        fstream(std::FILE* f)
            : file(f)
        {}

        virtual uint8_t next() override 
        {
            return std::fgetc(file);
        }
        
        virtual uint8_t peek() const override 
        {
            auto c = std::fgetc(file);
            std::ungetc(c, file);
            return c;
        }

        virtual std::string range(uint32_t start, uint32_t len) override 
        {
            auto pos = std::ftell(file);
            std::fseek(file, start, SEEK_SET);

            char* chars = (char*)alloca(len);
            std::fread(chars, 1, len, file);

            std::fseek(file, pos, SEEK_SET);

            return std::string(chars, len);
        }

        virtual ~fstream() override { std::fclose(file); }

    private:
        std::FILE* file;
    };

    struct sstream : stream
    {
        sstream(std::string* s)
            : str(s)
            , idx(0)
        {}

        virtual uint8_t next() override 
        {   
            if(idx >= str->size())
                return '\0';
                
            return str->at(idx++);
        }
        
        virtual uint8_t peek() const override 
        {
            if(idx >= str->size())
                return '\0';

            return str->at(idx);
        }

        virtual std::string range(uint32_t start, uint32_t len) override 
        {
            return std::string(&str->at(start), len);
        }

        virtual ~sstream() override { }

    private:
        std::string* str;
        int idx;
    };
}