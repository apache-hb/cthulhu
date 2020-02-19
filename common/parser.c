#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ctu_parser parser_alloc(ctu_lexer lex)
{
    ctu_parser parse = {
        .lex = lex
    };
    
    return parse;
}

void parser_free(ctu_parser* self)
{
    lexer_free(&self->lex);
}

static inline ctu_token nextt(ctu_parser* self)
{
    return lexer_next(&self->lex);
}

static inline ctu_token peekt(ctu_parser* self)
{
    return lexer_peek(&self->lex);
}

static keyword_e next_key(ctu_parser* self)
{
    ctu_token tok = nextt(self);
    
    // special case for eof
    if(tok.type == tt_eof)
    {
        return op_none;
    }

    if(tok.type != tt_keyword)
    {
        // ERROR
    }

    return tok.key;
}

static keyword_e peek_key(ctu_parser* self)
{
    ctu_token tok = peekt(self);
    return tok.type == tt_keyword ? tok.key : op_none;
}

static char* next_ident(ctu_parser* self)
{
    ctu_token tok = nextt(self);
    if(tok.type != tt_ident)
    {
        // ERROR
    }

    return tok.str;
}

static char* peek_ident(ctu_parser* self)
{
    ctu_token tok = peekt(self);
    return tok.type == tt_ident ? tok.str : 0;
}

static void expect_key(ctu_parser* self, keyword_e key)
{
    if(next_key(self) != key)
    {
        // error
    }
}

static int consume_key(ctu_parser* self, keyword_e key)
{
    if(peek_key(self) == key)
    {
        nextt(self);
        return 1;
    }

    return 0;
}

static ctu_node* make_node(node_type_e type)
{
    ctu_node* node = malloc(sizeof(ctu_node));
    node->type = type;
    return node;
}

static ctu_node* parse_type_decl(ctu_parser* self);

static ctu_node* parse_dotted_name(ctu_parser* self)
{
    ctu_node* node = make_node(nt_dotted_name_decl);

    // TODO: arbitrary length
    node->parts = malloc(sizeof(char*) * 16);
    node->count = 0;

    do {
        node->parts[node->count++] = next_ident(self);
    } while(consume_key(self, op_sep));

    node->parts = realloc(node->parts, sizeof(char*) * node->count);

    return node;
}

static ctu_node* parse_module_decl(ctu_parser* self)
{
    ctu_node* node = make_node(nt_module_decl);

    node->path = parse_dotted_name(self);

    return node;
}

static ctu_node* parse_import_decls(ctu_parser* self)
{
    ctu_node* node = make_node(nt_import_decl);

    // TODO: should be any length
    node->count = 0;
    node->imports = malloc(sizeof(ctu_node*) * 32);

    do {
        node->imports[node->count++] = parse_dotted_name(self);
    } while(consume_key(self, kw_import));

    node->imports = realloc(node->imports, sizeof(ctu_node*) * node->count);

    return node;
}

static ctu_node* parse_tuple_decl(ctu_parser* self)
{
    ctu_node* node = make_node(nt_tuple_decl);

    // TODO: size
    node->count = 0;
    node->fields = malloc(sizeof(ctu_node*) * 8);

    if(peek_key(self) != op_closearg)
    {
        do {
            node->fields[node->count++] = parse_type_decl(self);
        } while(consume_key(self, op_comma));
    }

    node->fields = realloc(node->fields, sizeof(ctu_node*) * node->count);

    return node;
}

static ctu_node* parse_struct_decl(ctu_parser* self)
{
    ctu_node* node = make_node(nt_struct_decl);

    // TODO
    node->count = 0;
    node->names = malloc(sizeof(char*) * 16);
    node->fields = malloc(sizeof(ctu_node*) * 16);

    // TODO: could clean this up
    if(peek_key(self) != op_closescope)
    {
        for(;;)
        {

            char* name = next_ident(self);

            expect_key(self, op_colon);

            ctu_node* type = parse_type_decl(self);

            node->names[node->count] = name;
            node->fields[node->count] = type;
            node->count++;

            if(!consume_key(self, op_comma))
                break;
        }
    }

    node->names = realloc(node->names, sizeof(char*) * node->count);
    node->fields = realloc(node->fields, sizeof(ctu_node*) * node->count);

    return node;
}

static ctu_node* parse_array_decl(ctu_parser* self)
{
    // TODO: this needs expression parsing
    return NULL;
}

static ctu_node* parse_typename_decl(ctu_parser* self)
{
    ctu_node* node = make_node(nt_typename_decl);

    node->path = parse_dotted_name(self);

    return node;
}

static ctu_node* parse_type_decl(ctu_parser* self)
{
    ctu_node* node = NULL;

    ctu_token tok = peekt(self);

    if(tok.type == tt_ident)
    {
        node = parse_typename_decl(self);
    }
    else if(tok.type == tt_keyword)
    {
        nextt(self);
        switch(tok.type)
        {
        case op_openarg:
            node = parse_tuple_decl(self);
            break;
        case op_openscope:
            node = parse_struct_decl(self);
            break;
        case op_openarr:
            node = parse_array_decl(self);
            break;
        default:
            // error
            break;
        }
    }

    if(consume_key(self, op_mul))
        node->count |= PTR_FLAG;

    return node;
}

static ctu_node* parse_using_decl(ctu_parser* self)
{
    ctu_node* node = make_node(nt_using_decl);

    node->name = next_ident(self);

    expect_key(self, op_assign);

    node->typedecl = parse_type_decl(self);

    return node;
}

static ctu_node* parse_func_decl(ctu_parser* self)
{
    return NULL;
}

static ctu_node* parse_body_decls(ctu_parser* self)
{
    ctu_node* node = make_node(nt_body_decl);

    // TODO: arbitrary size
    node->count = 0;
    node->decls = malloc(sizeof(ctu_node*) * 64);

    for(;;)
    {
        keyword_e key = next_key(self);

        // handle eof
        if(key == op_none)
            break;

        ctu_node* part = NULL;

        switch(key) 
        {
        case kw_def:
            part = parse_func_decl(self);
            break;
        case kw_using:
            part = parse_using_decl(self);
            break;
        default:
            // invalid keyword
            break;
        }

        node->decls[node->count++] = part;
    }

    return node;
}

// toplevel: [module] [import+] [body+]
ctu_node* parser_ast(ctu_parser* self)
{
    ctu_node* node = make_node(nt_toplevel);

    node->module_decl = NULL;
    node->import_decls = NULL;
    node->body_decls = NULL;

    if(consume_key(self, kw_module))
        node->module_decl = parse_module_decl(self);

    if(consume_key(self, kw_import))
        node->import_decls = parse_import_decls(self);

    node->body_decls = parse_body_decls(self);

    return node;
}
