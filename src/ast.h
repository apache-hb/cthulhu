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
    char* name;
    struct type_t* type;
} typepair_t;

typedef enum {
    _ASSIGN,
    _FOR,
    _WHILE,
    _BRANCH,
    _UNARY,
    _BINARY,
    _TERNARY,
    _LIST,
    LET_STMT,
    _NAME
} stmt_type;

typedef struct stmt_t {
    stmt_type type;

    union {
        struct {
            char* name;
            struct type_t* type;
            struct stmt_t* init;
        } _var;
    } data;
} stmt_t;

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
            
            map<char*, stmt_t> fields;
        } _enum;

        map<char*, type_t*> _union;

        map<char*, type_t*> _any;

        struct type_t* _const;

        struct {
            struct type_t* of;
            stmt_t* size;
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
    vec<typepair_t> args;
    type_t* ret;
    stmt_t* body;
} func_t;

typedef struct {
    char* name;
    func_t* func;
    stmt_t* body;
} funcpair_t;

typedef struct {
    vec<include_t> deps;
    map<char*, type_t*> types;
    map<char*, func_t*> funcs;
} ast_t;

#endif // AST_H
