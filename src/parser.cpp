#include "parser.h"

#include "token.h"

#include <stdio.h>
#include <string.h>

static type_t* parse_type(parser_t* self);

static token_t parser_next(parser_t* self)
{
    token_t tok = self->tok;
    self->tok.type = token_t::INVALID;
    if(tok.type == token_t::INVALID)
    {
        tok = lexer_next(&self->source);
    }

    return tok;
}

static int parser_consume(parser_t* self, keyword key)
{
    token_t tok = parser_next(self);

    if(tok.type != token_t::KEYWORD || tok.data.key != key)
    {
        self->tok = tok;
        return 0;
    }
    else
    {
        return 1;
    }
}

static char* consume_ident(parser_t* self)
{
    token_t tok = parser_next(self);

    if(tok.type != token_t::IDENT)
    {
        self->tok = tok;
        return NULL;
    }

    return tok.data.ident;
}

static void expect_key(parser_t* self, keyword key)
{
    token_t tok = parser_next(self);

    if(tok.type != token_t::KEYWORD || tok.data.key != key)
    {
        printf("unexpected keyword\n");
    }
}

static char* expect_ident(parser_t* self)
{
    token_t tok = parser_next(self);
    if(tok.type != token_t::IDENT)
    {
        return NULL;
    }
    else
    {
        return tok.data.ident;
    }
}

static vec<char*> parse_path(parser_t* self)
{
    vec<char*> out;
    
    do 
    { 
        char* part = expect_ident(self);
        printf("::%s", part);
        out.push_back(part);
    }
    while(parser_consume(self, COLON2));
    
    return out;
}

static include_t parse_include(parser_t* self)
{
    include_t out;
    out.path = parse_path(self);
    out.alias = parser_consume(self, ARROW) ? expect_ident(self) : NULL;
    printf(" -> %s\n", out.alias);
    return out;
}

static type_t* make_type(typetype t)
{
    type_t* out = (type_t*)malloc(sizeof(type_t));
    out->type = t;
    return out;
}

static expr_t* parse_expr(parser_t* self)
{
    (void)self;
    return NULL;
}

static type_t* make_ptr(type_t* type)
{
    type_t* out = make_type(_PTR);
    out->data._ptr = type;
    return out;
}

static type_t* make_ref(type_t* type)
{
    type_t* out = make_type(_REF);
    out->data._ref = type;
    return out;
}

static type_t* make_array(parser_t* self, type_t* type)
{
    type_t* out = make_type(_ARRAY);
    out->data._array.of = type;
    out->data._array.size = parse_expr(self);
    expect_key(self, RSQUARE);
    return out;
}

static type_t* make_builtin(builtin t)
{
    type_t* out = make_type(_BUILTIN);
    out->data._builtin = t;
    return out;
}

static type_t* builtin_u8 = make_builtin(_U8);
static type_t* builtin_u16 = make_builtin(_U16);
static type_t* builtin_u32 = make_builtin(_U32);
static type_t* builtin_u64 = make_builtin(_U64);

static type_t* builtin_i8 = make_builtin(_I8);
static type_t* builtin_i16 = make_builtin(_I16);
static type_t* builtin_i32 = make_builtin(_I32);
static type_t* builtin_i64 = make_builtin(_I64);

static type_t* builtin_f32 = make_builtin(_F32);
static type_t* builtin_f64 = make_builtin(_F64);

static type_t* builtin_void = make_builtin(_VOID);

static type_t* builtin_or_name(char* name)
{
    type_t* out;

    if(strcmp(name, "i8") == 0)
    {
        out = builtin_i8;
    }
    else if(strcmp(name, "i16") == 0)
    {
        out = builtin_i16;
    }
    else if(strcmp(name, "i32") == 0) 
    {
        out = builtin_i32;
    }
    else if(strcmp(name, "i64") == 0) 
    {
        out = builtin_i64;
    }
    else if(strcmp(name, "u8") == 0) 
    {
        out = builtin_u8;
    }
    else if(strcmp(name, "u16") == 0) 
    {
        out = builtin_u16;
    }
    else if(strcmp(name, "u32") == 0) 
    {
        out = builtin_u32;
    }
    else if(strcmp(name, "u64") == 0) 
    {
        out = builtin_u64;
    }
    else if(strcmp(name, "f32") == 0) 
    {
        out = builtin_f32;
    }
    else if(strcmp(name, "f64") == 0) 
    {
        out = builtin_f64;
    }
    else if(strcmp(name, "void") == 0) 
    {
        out = builtin_void;
    }
    else
    {
        out = make_type(_NAME);
        out->data._path.push_back(name);
    }

    return out;
}

static type_t* parse_typepath(char* init, parser_t* self)
{
    vec<char*> path;
    path.push_back(init);

    while(parser_consume(self, COLON2))
    {
        path.push_back(expect_ident(self));
    }

    if(path.size() == 1)
    {
        return builtin_or_name(path[0]);
    }
    else
    {
        type_t* out = make_type(_NAME);
        out->data._path = path;
        return out;
    }
}

static type_t* parse_struct(parser_t* self)
{
    type_t* out = make_type(_STRUCT);

    expect_key(self, LBRACE);

    vec<typepair_t> fields;

    char* field = consume_ident(self);
    while(field)
    {
        expect_key(self, COLON);
        type_t* type = parse_type(self);
        fields.push_back({ field, type });

        field = consume_ident(self);
    }

    expect_key(self, RBRACE);

    out->data._struct = fields;

    return out;
}

static type_t* parse_union(parser_t* self)
{
    type_t* out = make_type(_UNION);

    map<char*, type_t*> fields;
    
    expect_key(self, LBRACE);
    
    char* field = consume_ident(self);
    while(field)
    {
        expect_key(self, COLON);
        type_t* type = parse_type(self);
        fields[field] = type;

        field = consume_ident(self);
    }

    expect_key(self, RBRACE);

    out->data._union = fields;

    return out;
}

static type_t* parse_any(parser_t* self)
{
    type_t* out = make_type(_ANY);

    map<char*, type_t*> fields;
    
    expect_key(self, LBRACE);
    
    char* field = consume_ident(self);
    while(field)
    {
        expect_key(self, BIGARROW);
        type_t* type = parse_type(self);
        fields[field] = type;

        field = consume_ident(self);
    }

    expect_key(self, RBRACE);

    out->data._any = fields;

    return out;
}

static type_t* parse_const(parser_t* self)
{
    expect_key(self, LT);
    type_t* type = parse_type(self);
    expect_key(self, GT);

    type_t* out = make_type(_CONST);
    out->data._const = type;

    return out;
}

static type_t* parse_enum(parser_t* self)
{
    type_t* out = make_type(_ENUM);

    map<char*, type_t*> fields;
    
    expect_key(self, LBRACE);
    
    char* field = consume_ident(self);
    while(field)
    {
        expect_key(self, BIGARROW);
        type_t* type = parse_type(self);
        fields[field] = type;

        field = consume_ident(self);
    }

    expect_key(self, RBRACE);

    out->data._union = fields;

    return out;
}

static type_t* parse_closure(parser_t* self)
{
    type_t* out = make_type(_CLOSURE);
    
    vec<type_t*> args;

    for(;;)
    {
        if(parser_consume(self, RPAREN))
        {
            break;
        }
        type_t* arg = parse_type(self);
        args.push_back(arg);
        expect_key(self, COMMA);
    }

    expect_key(self, ARROW);
    type_t* ret = parse_type(self);

    out->data._closure.args = args;
    out->data._closure.ret = ret;

    return out;
}

static type_t* parse_type(parser_t* self)
{
    token_t tok = parser_next(self);
    type_t* out;

    if(tok.type == token_t::KEYWORD)
    {
        if(tok.data.key == STRUCT)
        {
            out = parse_struct(self);
        }
        else if(tok.data.key == UNION)
        {
            out = parse_union(self);
        }
        else if(tok.data.key == ENUM)
        {
            out = parse_enum(self);
        }
        else if(tok.data.key == ANY)
        {
            out = parse_any(self);
        }
        else if(tok.data.key == CONST)
        {
            out = parse_const(self);
        }
        else if(tok.data.key == LPAREN)
        {
            out = parse_closure(self);
        }
        else
        {
            printf("oh no\n");
            // error
        }
    }
    else if(tok.type == token_t::IDENT)
    {
        out = parse_typepath(tok.data.ident, self);
    }
    else
    {
        printf("oh no 2\n");
        // error
    }

    for(;;)
    {
        if(parser_consume(self, MUL))
        {
            out = make_ptr(out);
        }
        else if(parser_consume(self, AND))
        {
            out = make_ref(out);
        }
        else if(parser_consume(self, LSQUARE))
        {
            out = make_array(self, out);
        }
        else
        {
            break;
        }
    }

    return out;
}

static typepair_t parse_typedef(parser_t* self)
{
    char* name = expect_ident(self);
    typepair_t pair;
    pair.name = name;

    expect_key(self, ASSIGN);

    pair.type = parse_type(self);

    return pair;
}

ast_t produce_ast(parser_t* self)
{
    vec<include_t> includes;

    while(parser_consume(self, INCLUDE))
    {
        printf("include ");
        include_t inc = parse_include(self);
        includes.push_back(inc);
    }

    map<char*, type_t*> types;

    for(;;)
    {
        if(parser_consume(self, TYPE))
        {
            typepair_t type = parse_typedef(self);
            types[type.name] = type.type;
        }
        else
        {
            break;
        }
    }

    ast_t ast;
    ast.deps = includes;
    ast.types = types;

    return ast;
}

parser_t parser_new(lexer_t source)
{
    parser_t out;
    out.source = source;
    out.tok.type = token_t::INVALID;
    return out;
}