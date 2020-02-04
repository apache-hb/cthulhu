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

static token_t nexttok(parser_t* self)
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
    free(self);
}

///////////////////////////////////////////////////
///                lexer interface
///////////////////////////////////////////////////

static keyword_t next_keyword(parser_t* self)
{
    token_t tok = lexer_next(self);
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

static char* next_ident(parser_t* self)
{
    token_t tok = lexer_next(self);
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

// type-name: ident
static node_t* parse_type_name(parser_t* self)
{
    node_t* node = make_node(name_decl);
    node->name = next_ident(self);
    return node;
}

// named-type-list-decl: ident `:` type-decl [`,` named-type-list-decl] [`,`]
static node_t* parse_named_type_list_decl(parser_t* self)
{

}

// type-list-decl: type-decl [`,` type-list-decl] [`,`]
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

}

key_func_pair_t type_table[] = {
    { op_openscope, parse_struct_decl },
    { op_openarg, parse_tuple_decl },
    { op_openarr, parse_array_decl },
    { op_func, parse_func_type_decl }
};

static const type_table_len = sizeof(type_table) / sizeof(key_func_pair_t);


// type-decl: (struct-decl | tuple-decl | array-decl | func-type-decl | type-name)[`?` | `*`]
static node_t* parse_type_decl(parser_t* self)
{
    node_t* node;


}

// using-decl: `using` [ident `=` type-decl | `scope` dotted-name | `module` dotted-name]
static node_t* parse_using_decl(parser_t* self)
{
    token_t tok = lexer_next(self);

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

static const body_table_len = sizeof(body_table) / sizeof(key_func_pair_t);

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