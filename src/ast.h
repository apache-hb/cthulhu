#pragma once

#include <vector>
#include <string>
#include <map>

namespace ct::ast {
    using namespace std;

    struct Import {
        vector<string> path;
        vector<string> items;
    };

    struct Type { };

    struct Builtin : Type {
        enum {
            // well defined sizes
            U8,
            U16,
            U32,
            U64,
            I8,
            I16,
            I32,
            I64,
            F32,
            F64,
            VOID,

            // undefined sizes
            BOOL,
            CHAR,
            UCHAR,
            SHORT,
            USHORT,
            INT,
            UINT,
            LONG,
            ULONG,
            FLOAT,
            DOUBLE,
            ISIZE,
            USIZE
        } type;

        using type_t = decltype(type);
        constexpr Builtin(type_t t)
            : type(t)
        { }
    };

    struct Name : Type {
        using data_t = std::vector<std::string>;
        Name(data_t d)
            : path(d)
        { }
        data_t path;
    };

    struct Struct : Type {
        vector<pair<string, Type*>> fields;
    };

    struct Body {
        map<string, Type*> types;
    };

    struct Program {
        vector<Import> imports;
        Body body;
    };
}