#pragma once

#include <stdint.h>

#include <string>

namespace ctu
{
    struct FilePos
    {
        uint64_t pos;
        uint64_t line;
        uint64_t col;
    };

    struct Stream
    {
        virtual ~Stream() {}
        virtual uint8_t next() = 0;
        virtual uint8_t peek() const = 0;

        virtual std::string range(uint32_t start, uint32_t len) = 0;
    };

    struct FStream : Stream
    {
        FStream(std::FILE* f)
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

        virtual ~FStream() override { std::fclose(file); }

    private:
        std::FILE* file;
    };

    struct SStream : Stream
    {
        SStream(std::string* s)
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

        virtual ~SStream() override { }

    private:
        std::string* str;
        int idx;
    };
}