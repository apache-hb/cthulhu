#include "common.h"

#include <stdlib.h>

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

static void expect(parser_t* self, keyword_t key)
{
    token_t tok = nexttok(self);
    if(tok.type != keyword && tok.key != key)
    {
        // TODO:
        printf("expected keyword (key)\n");
    }
}

static keyword_t next_keyword(parser_t* self)
{
    token_t tok = nexttok(self);
    if(tok.type != keyword)
    {
        // TODO
    }
    return tok.key;
}

static char* next_ident(parser_t* self)
{
    token_t tok = nexttok(self);
    if(tok.type != ident)
    {
        printf("expected ident\n");
        return "";
    }
    return tok.str;
}

static type_t* delayed_type()
{
    type_t* t = malloc(sizeof(type_t));
    t->type = delayed;
    return t;
}

static struct node_t* parse_body(parser_t* self);

static struct node_t* parse_type(parser_t* self)
{
    token_t tok = nexttok(self);
    // if the keyword is {} then its a struct
    // if its () its a tuple
    // if its a [] then its an array
    // otherwise its a typename

    if(tok.type == keyword)
    {

    }
    else
    {
        // is a typename
        node_t* node = malloc(sizeof(node_t));
        node->node_type = type_decl;
        // TODO
        node->type_name = parse_type_name(tok);
    }
    return NULL;
}

static struct node_t* parse_func(parser_t* self)
{
    return NULL;
}

static struct node_t* parse_using(parser_t* self)
{
    return NULL;
}

static struct node_t* parse_scope(parser_t* self)
{
    node_t* node = malloc(sizeof(node_t));
    node->node_type = scope_decl;
    node->scope_name = next_ident(self);

    expect(self, op_openscope);

    node->content = parse_body(self);

    expect(self, op_closescope);

    return (struct node_t*)node;
}

static struct node_t* parse_body(parser_t* self)
{
    keyword_t tok = next_keyword(self);
    switch(tok)
    {
        case kw_def:
            return parse_func(self);
        case kw_scope:
            return parse_scope(self);
        case kw_using:
            return parse_using(self);
        default:
            printf("oh no\n");
            break;
    }
    return NULL;
}

node_t* parser_generate_ast(parser_t* self)
{
    token_t tok = nexttok(self);

    // we allow empty files
    if(tok.type == eof)
        return NULL;

    // the first token has to be a keyword
    if(tok.type != keyword)
    {
        printf("invalid file, keyword not first token\n");
        return NULL;
    }

    char* name;

    switch(tok.key)
    {
        case kw_def:
            // func decl
            break;
        case kw_scope:
            name = next_ident(self);
            node_t node = { scope_decl };
            node.scope_name = name;
            // scope decl
            break;
        case kw_using:
            // struct/typealias/tuple decl
            break;

        default:
            // invalid
            break;
        // function
    }

    return NULL;
}