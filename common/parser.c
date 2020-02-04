#include "common.h"

#include <stdlib.h>
#include <string.h>

parser_t* parser_alloc(lexer_t* lex)
{
    parser_t* self = malloc(sizeof(parser_t));
    self->source = lex;

    return self;
}

void parser_free(parser_t* self)
{
    lexer_free(self->source);
    free(self);
}

static token_t next_tok(parser_t* self)
{
    return lexer_next(self->source);
}

typedef struct {
    keyword_t key;
    node_t*(*func)(parser_t*);
} key_func_pair_t;

static node_t* make_node(node_type_t type)
{
    node_t* node = malloc(sizeof(node_t));
    node->node_type = type;
    return node;
}

static node_t* str_node(char* str)
{
    node_t* node = make_node(name_decl);
    node->name = str;
    return node;
}

void node_free(node_t* self)
{
    // TODO: theres going to be more stuff here
    free(self);
}

///////////////////////////////////////////////////
///                lexer interface
///////////////////////////////////////////////////

static keyword_t next_keyword(parser_t* self)
{
    token_t tok = next_tok(self);
    if(tok.type != keyword)
    {
        // TODO: proper handling
        printf("expected keyword got %d instead\n", tok.type);
        exit(5);
    }
    keyword_t key = tok.key;
    
    token_free(tok);

    return key;
}

static keyword_t peek_keyword(parser_t* self)
{
    token_t tok = lexer_peek(self->source);
    if(tok.type != keyword)
    {
        // TODO
        printf("expected keyword got %d instead\n", tok.type);
        exit(9);
    }

    return tok.key;
}

static char* next_ident(parser_t* self)
{
    token_t tok = next_tok(self);
    if(tok.type != ident)
    {
        printf("expected ident got %d instead\n", tok.type);
        exit(6);
    }
    char* str = strdup(tok.str);
    token_free(tok);
    return str;
}

///////////////////////////////////////////////////
///               type parsing
///////////////////////////////////////////////////

static node_t* parse_type_decl(parser_t* self);

// type-name: ident
static node_t* parse_type_name(parser_t* self)
{
    node_t* node = make_node(name_decl);
    node->name = next_ident(self);
    return node;
}

// named-type-list-decl: ident `:` type-decl [`,` named-type-list-decl]
static node_t* parse_named_type_list_decl(parser_t* self)
{
    
}

// type-list-decl: type-decl [`,` type-list-decl]
static node_t* parse_type_list_decl(parser_t* self)
{
    
}

// func-type-decl: `&(` [type-list-decl] `)` `->` type-decl
static node_t* parse_func_type_decl(parser_t* self)
{

}

// array-decl: `[` type-decl `:` number `]`
// assert(number > 0)
static node_t* parse_array_decl(parser_t* self)
{

}


// tuple-decl: `(` [type-list-decl] `)`
static node_t* parse_tuple_decl(parser_t* self)
{

}


// struct-decl: `{` [named-type-list-decl] `}`
static node_t* parse_struct_decl(parser_t* self)
{
    int count = 0;
    node_t* node = make_node(struct_body_decl);

    node_t* fields = malloc(sizeof(node_t));

    for(;;)
    {
        token_t tok = next_tok(self);
        if(tok.type != ident)
            goto end;

        node_t* pair = parse_named_type_list_decl(self);

        memcpy(fields + (count * sizeof(node_t)), )
    }

end: node->field_count = count;
    node->fields
}

#if 0

// typed-union-body-decl: ident `->` type-decl [`,` typed-union-body-decl]
static node_t* parse_typed_union_body(parser_t* self)
{

}

// typed-union-decl: `{` [typed-union-body-decl] `}`
static node_t* parse_typed_union_decl(parser_t* self)
{

}

// enum-body-decl: ident (`:` number) [`,` enum-body-decl]
static node_t* parse_enum_body_decl(parser_t* self)
{

}

#endif

// union-decl: `union` ((tuple-decl | struct-decl) | `enum` [`:` type-decl] typed-union-decl)
static node_t* parse_union_decl(parser_t* self)
{
    
}

// enum-decl: `enum` [`:` type-decl] enum-body-decl
static node_t* parse_enum_decl(parser_t* self)
{
    // when parsing an enum we need to be aware that enums can have any of
    //  1. a backing type for defining values
    //      - this defaults to u32
    //      - can be any type with a well defined type size
    //      - in reality this means anything aside from optinal types
    //  2. key value pairs for each field
    //      - a key val pair can have any constant expression
    //      - `name: 0` is just as valid as `name: (1 << 0)` or `name: (constant_value + 5)`
    //      - we have to be able to account for all this
    //
}


key_func_pair_t type_table[] = {
    { op_openscope, parse_struct_decl },
    { op_openarg, parse_tuple_decl },
    { op_openarr, parse_array_decl },
    { op_func, parse_func_type_decl },
    { kw_enum, parse_enum_decl },
    { kw_union, parse_union_decl }
};

static const int type_table_len = sizeof(type_table) / sizeof(key_func_pair_t);


// type-decl: (struct-decl | tuple-decl | array-decl | func-type-decl | enum-decl | union-decl | type-name)[`?` | `*`]
static node_t* parse_type_decl(parser_t* self)
{
    // the type alias syntax is pretty extensive
    // we need to account for
    // 1. structs
    //  - these are declared with `{` `}`
    //  - each field is a type mapped to a name
    //
    // 2. tuples
    //  - these are declared with `(` `)`
    //  - each field is a type, no name is required
    //
    // 3. arrays
    //  - these are declared with `[` `]`
    //  - arrays must always have their size declared at compile time
    //    unless the data is also declared at compile time
    //
    // 4. function signatures
    //  - these are declared with `&(` type-decl... `)` `->` type-decl 
    //  - the args are a tuple of types
    //
    // 5. enums
    //  - these enums are backed by a type
    //  - the default backing type is u32 on 32 bit and u64 on 64 bit
    //  - the backing type can be any type with a well defined size
    //    this means that any type can be used as a backing type
    //    not that you should actually do that.
    //
    // 6. unions
    //  - the size of the union is the size of the largest type in the union
    //
    // 7. typesafe union
    //  - a typesafe union is an enum with data associated to each enum value
    //  - the size of a typesafe union is the size of the union + the size of the enum
    //
    // 8. optional values
    //  - this is any type-decl followed by `?`
    //  - the size of this is garunteed to be the size of the type 
    //    rounded up to the nearest multiple of 4 on 32 bit systems
    //    and rounded up to 8 on 64 bit systems
    //
    // 9. pointers
    //  - this is any type-decl followed by `*`
    //  - all pointers are implicityly optional due to `null`
    //  - on 32 bit systems this will be 4 bytes wide
    //  - on 64 bit systems this will be 8 bytes wide
    //

    // first token will be either a ident for a typename alias
    // or a keyword for either a struct, tuple, array, enum or union decl
    token_t tok = lexer_peek(self->source);

    // if it is an ident then it must be a typename
    if(tok.type == ident)
    {
        return parse_type_name(self);
    }
    else if(tok.type == keyword)
    {
        // must be either struct-decl tuple-decl array-decl func-type-decl enum-decl or union-decl
        for(int i = 0; i < type_table_len; i++)
        {
            if(type_table[i].key == tok.key)
            {
                // skip the peek'd keyword
                token_free(next_tok(self));
                // then parse
                return type_table[i].func(self);
            }
        }

        // if we get here it was an invalid keyword
    }
    else
    {
        // error here
    }
}

// using-decl: `using` [ident `=` type-decl | `scope` dotted-name | `module` dotted-name]
static node_t* parse_using_decl(parser_t* self)
{
    token_t tok = next_tok(self);

    if(tok.type == keyword)
    {
        // must be either `scope` or `module`
    }
    else if(tok.type == ident)
    {
        // must be a type declaration
        node_t* node = make_node(type_decl);
        node->type_name = str_node(next_ident(self));
        
        keyword_t key = next_keyword(self);
        if(key != op_assign)
        {
            printf("expected `=` got %d instead\n", key);
            exit(8);
        }

        node->type_data = parse_type_decl(self);

        return node;
    }
    else
    {
        // error
        printf("invalid using structure\n");
        exit(7);
    }
}

////////////////////////////////////////////////////
///             variable parsing
///////////////////////////////////////////////////

static node_t* parse_val_decl(parser_t* self)
{

}

static node_t* parse_var_decl(parser_t* self)
{

}

////////////////////////////////////////////////////
///             function parsing
////////////////////////////////////////////////////

static node_t* parse_func_decl(parser_t* self)
{

}

static key_func_pair_t body_table[] = {
    { kw_using, parse_using_decl },
    { kw_def, parse_func_decl },
    { kw_val, parse_val_decl },
    { kw_var, parse_var_decl }
};

static const int body_table_len = sizeof(body_table) / sizeof(key_func_pair_t);

// body-decl: (using-decl | func-decl | var-decl)+
static node_t* parse_body(parser_t* self)
{
    keyword_t key = next_keyword(self);
    
    for(int i = 0; i < body_table_len; i++)
    {
        if(body_table[i].key == key)
        {
            return body_table[i].func(self);
        }
    }

    // TODO: error here
    return NULL;
}

node_t* parser_generate_ast(parser_t* self)
{
    return NULL;
}