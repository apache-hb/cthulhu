#ifndef AST_H
#define AST_H

#include <vector>
#include <map>

using namespace std;

template<typename T>
using vec = vector<T>;

typedef struct {
    vec<char*> path;
    char* alias;
} include_t;

typedef struct {
    void* TODO;
} expr_t;

typedef struct {
    char* name;
    struct type_t* type;
} typepair_t;

typedef enum {
    _I8,
    _I16,
    _I32,
    _I64,
    _U8,
    _U16,
    _U32,
    _U64,
    _VOID,
    _F32,
    _F64
} builtin;

typedef enum {
    _STRUCT,
    _ENUM,
    _UNION,
    _ANY,
    _CONST,
    _ARRAY,
    _PTR,
    _REF,
    _CLOSURE,
    _BUILTIN,
    _NAME
} typetype;

typedef struct type_t {
    typetype type;

    union {
        vec<typepair_t> _struct;

        struct {
            struct type_t* backing;
            
            map<char*, expr_t> fields;
        } _enum;

        map<char*, type_t*> _union;

        map<char*, type_t*> _any;

        struct type_t* _const;

        struct {
            struct type_t* of;
            expr_t* size;
        } _array;

        struct type_t* _ptr;

        struct type_t* _ref;

        struct {
            vec<type_t*> args;

            struct type_t* ret;
        } _closure;

        
        builtin _builtin;

        vec<char*> _path;
    } data;
} type_t;

typedef struct {
    vec<include_t> deps;
    map<char*, type_t*> types;
} ast_t;

#endif // AST_H
