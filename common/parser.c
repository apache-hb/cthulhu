#include "common.h"

#include <stdlib.h>

parser_t parser_alloc(lexer_t lex)
{
    parser_t parse = {
        .lex = lex
    };
    
    return parse;
}

void parser_free(parser_t* self)
{
    lexer_free(&self->lex);
}

static inline token_t nextt(parser_t* self)
{
    return lexer_next(&self->lex);
}

static inline token_t peekt(parser_t* self)
{
    return lexer_peek(&self->lex);
}

static keyword_e next_key(parser_t* self)
{
    token_t tok = nextt(self);
    
    if(tok.type != tt_keyword)
    {
        // ERROR
    }

    return tok.key;
}

static keyword_e peek_key(parser_t* self)
{
    token_t tok = peekt(self);
    return tok.type == tt_keyword ? tok.key : op_none;
}

static char* next_ident(parser_t* self)
{
    token_t tok = nextt(self);
    if(tok.type != tt_ident)
    {
        // ERROR
    }

    return tok.str;
}

static char* peek_ident(parser_t* self)
{
    token_t tok = peekt(self);
    return tok.type == tt_ident ? tok.str : 0;
}

static void expect_key(parser_t* self, keyword_e key)
{
    if(next_key(self) != key)
    {
        // error
    }
}

static node_t* make_node(node_type_e type)
{
    node_t* node = malloc(sizeof(node_t));
    node->type = type;
    return node;
}

static dotted_name_t parse_dotted_name(parser_t* self)
{
    char** names = malloc(sizeof(char*) * 16);
    int count = 0;

    for(;;)
    {
        names[count++] = next_ident(self);
        
        if(peek_key(self) != op_sep)
        {
            break;
        }

        expect_key(self, op_sep);
    }

    dotted_name_t name = {
        .num = count,
        .parts = realloc(names, sizeof(char*) * count)
    };

    return name;
}

static module_t parse_module(parser_t* self)
{
    module_t mod = {
        .path = parse_dotted_name(self)
    };

    return mod;
}

static import_t parse_import(parser_t* self)
{
    import_t imp = {
        .path = parse_dotted_name(self)
    };

    return imp;
}

toplevel_t parser_ast(parser_t* self)
{
    keyword_e key = next_key(self);

    toplevel_t top;

    // [`module` dotted-name]
    if(key == kw_module)
    {
        top.mod = parse_module(self);
        key = next_key(self);
    }

    // [`import` dotted-name]+
    while(key == kw_import)
    {
        int count = 0;
        import_t* imports = malloc(sizeof(import_t) * 16);

        imports[count++] = parse_import(self);

        key = next_key(self);
    }

    
}

#if 0

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

static token_t peek_tok(parser_t* self)
{
    return lexer_peek(self->source);
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

static void expect_keyword(parser_t* self, keyword_t key)
{
    keyword_t k = next_keyword(self);
    if(k != key)
    {
        printf("expected %d got %d\n", key, k);
        exit(11);
    }
}

static keyword_t peek_keyword(parser_t* self)
{
    token_t tok = peek_tok(self);

    if(tok.type != keyword)
        return kw_none;

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

// forward declares
static node_t* parse_type_decl(parser_t* self);
static node_t* parse_body(parser_t* self);

///////////////////////////////////////////////////
///               expr parsing
///////////////////////////////////////////////////

static node_t* parse_expr(parser_t* self)
{

}

///////////////////////////////////////////////////
///               type parsing
///////////////////////////////////////////////////

// dotted-name-decl: ident [`::` dotted-name-decl]
static node_t* parse_dotted_name_decl(parser_t* self)
{
    name_stack_t names = { 0, malloc(sizeof(char*) * 16) };

    int count = 0;
    for(;;)
    {
        // ident
        char* name = next_ident(self);
        name_stack_push(&names, name);
        count++;
        printf("%s", name);

        // `::` scope-name-decl
        if(peek_keyword(self) != op_sep)
        {
            break;
        }
        next_keyword(self);
        printf("::");
    }

    node_t* node = make_node(dotted_name_decl);

    node->dotted_name_decl.count = count;
    node->dotted_name_decl.parts = realloc(names.names, sizeof(char*) * count);

    return node;
}


// name: dotted-name
static node_t* parse_type_name_decl(parser_t* self)
{
    return parse_dotted_name_decl(self);
}

// named-type-list-decl: ident `:` type-decl [`,` named-type-list-decl]
static node_t* parse_named_type_list_decl(parser_t* self)
{
    // TODO: this needs to resize itself
    node_stack_t stack = { 0, malloc(sizeof(node_t) * 64) };

    for(;;)
    {
        // [`,` named-type-list-decl]
        token_t tok = peek_tok(self);
        if(tok.type != ident)
        {
            break;
        }

        node_t* node = make_node(field_decl);

        // ident
        node->field_decl.name = next_ident(self);
        printf("%s", node->field_decl.name);

        // `:`
        expect_keyword(self, op_colon);
        printf(": ");

        node->field_decl.type = parse_type_decl(self);

        node_stack_push(&stack, node);

        if(peek_keyword(self) != op_comma)
        {
            break;
        }
        next_tok(self);
        printf(", ");
    }

    // expect_keyword(self, op_closearg);

    node_t* node = make_node(named_type_list_decl);

    node->named_type_list_decl.count = stack.top;
    node->named_type_list_decl.types = realloc(stack.nodes, sizeof(node_t) * stack.top);

    return node;
}

// type-list-decl: type-decl [`,` type-list-decl]
static node_t* parse_type_list_decl(parser_t* self)
{
    // TODO: this needs to resize itself
    node_stack_t stack = { 0, malloc(sizeof(node_t) * 64) };

    for(;;)
    {
        if(peek_keyword(self) == op_closearg)
        {
            break;
        }
        // type-decl
        node_stack_push(&stack, parse_type_decl(self));

        // [`,` type-list-decl]
        if(peek_keyword(self) != op_comma)
        {
            break;
        }

        next_tok(self);
        printf(", ");
    }

    expect_keyword(self, op_closearg);

    node_t* node = make_node(type_list_decl);
    node->type_list_decl.count = stack.top;
    node->type_list_decl.types = realloc(stack.nodes, sizeof(node_t) * stack.top);
    return node;
}

// func-type-decl: `&(` [type-list-decl] `)` `->` type-decl
static node_t* parse_func_type_decl(parser_t* self)
{
    node_t* node = make_node(func_sig_decl);

    node->func_sig_decl.args = parse_type_list_decl(self);
    expect_keyword(self, op_arrow);
    node->func_sig_decl.ret_type = parse_type_decl(self);

    return node;
}

// array-decl: `[` type-decl [`:` number] `]`
// assert(number > 0)
static node_t* parse_array_decl(parser_t* self)
{
    node_t* node = make_node(array_decl);
    node->array_decl.type = parse_type_decl(self);

    if(peek_keyword(self) != op_colon)
    {
        node->array_decl.size = NULL;
    }
    else
    {
        printf(":");
        next_tok(self);
        node->array_decl.size = parse_expr(self);
    }

    // skip the `]`
    next_tok(self);

    return node;
}

static node_t* parse_enum_prelude_decl(parser_t* self)
{
    printf("enum");
    keyword_t key = peek_keyword(self);

    if(key == op_colon)
    {
        printf(": ");
        next_keyword(self);
        return parse_type_decl(self);
    }

    return NULL;
}

// enum-body-decl: ident (`:` number) [`,` enum-body-decl]
static node_t* parse_enum_body_decl(parser_t* self)
{
    node_t* node = make_node(enum_decl);
    node_stack_t nodes = { 0, malloc(sizeof(node_t) * 16) };
    for(;;)
    {
        token_t tok = next_tok(self);
        if(tok.type != ident)
        {
            break;
        }

        node_t* pair = make_node(enum_field_decl);
        pair->field_decl.name = tok.str;

        expect_keyword(self, op_colon);

        pair->field_decl.type = parse_expr(self);

        node_stack_push(&nodes, pair);

        if(peek_keyword(self) != op_comma)
        {
            next_tok(self);
            break;
        }
    }

    return node;
}

// enum-decl: enum-prelude enum-body-decl
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

    node_t* node = make_node(enum_decl);
    node->typed_enum_decl.backing = parse_enum_prelude_decl(self);

    keyword_t key = next_keyword(self);

    if(key == op_openarg)
    {
        printf("(");
        name_stack_t stack = { 0, malloc(sizeof(char*) * 16) };

        for(;;)
        {
            token_t tok = next_tok(self);
            if(tok.type != ident)
            {
                printf(")");
                break;
            }
            printf("%s", tok.str);
            name_stack_push(&stack, tok.str);

            // next_tok(self);

            if(peek_keyword(self) == op_comma)
            {
                printf(", ");
                next_tok(self);
                continue;
            }
        }

        node->typed_enum_decl.field_count = stack.top;
        node->typed_enum_decl.fields = realloc(stack.names, sizeof(char*) * stack.top);
    }
    else if(key == op_openscope)
    {
        printf("{");

        node_t* node = parse_enum_body_decl(self);
        node->node_type = union_decl;

        printf("}");
    }

    return node;
}

static node_t* parse_typed_enum_field_decl(parser_t* self)
{
    node_t* field = make_node(field_decl);

    field->field_decl.name = next_ident(self);
    printf("%s ", field->field_decl.name);

    expect_keyword(self, op_arrow);
    printf("-> ");

    field->field_decl.type = parse_type_decl(self);

    return field;
}

// union-decl: `union` ((type-list-decl | struct-decl) | `enum` [`:` type-decl] typed-union-decl)
static node_t* parse_union_decl(parser_t* self)
{
    printf("union ");
    keyword_t key = next_keyword(self);
    if(key == kw_enum)
    {
        node_t* node = make_node(typed_enum_decl);
        node_t* backing = parse_enum_prelude_decl(self);

        key = next_keyword(self);

        if(key != op_openscope)
        {
            // TODO: this
            printf("error\n");
        }

        node_stack_t stack = { 0, malloc(sizeof(node_t) * 16) };

        printf(" { ");

        for(;;)
        {
            if(peek_keyword(self) == op_closescope)
            {
                next_tok(self);
                break;
            }

            node_stack_push(&stack, parse_typed_enum_field_decl(self));

            if(next_keyword(self) != op_comma)
            {
                break;
            }

            printf(", ");
        }

        printf(" }");

        node->typed_enum_decl.backing = backing;
        node->typed_enum_decl.field_count = stack.top;
        node->typed_enum_decl.fields = realloc(stack.nodes, sizeof(node_t) * stack.top);
        return node;
    }
    else if(key == op_openscope)
    {
        printf("{ ");
        node_t* fields = parse_named_type_list_decl(self);
        fields->node_type = union_decl;
        expect_keyword(self, op_closescope);
        printf(" }");
        return fields;
    }
    else if(key == op_openarg)
    {
        printf("( ");
        node_t* fields = parse_type_list_decl(self);
        fields->node_type = union_tuple_decl;
        // expect_keyword(self, op_closearg);
        printf(" )");
        return fields;
    }
    else
    {
        // ERROR
    }
}

// type-decl: (struct-decl | union-decl | array-decl | func-type-decl | enum-decl | union-decl | type-name)[`*`]
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

    token_t tok = peek_tok(self);

    node_t* node = make_node(type_decl);

    if(tok.type == ident)
    {
        // type-name
        node->type_decl.data = parse_type_name_decl(self);
    }
    else if(tok.type == keyword)
    {
        next_tok(self);
        // (struct-decl | type-list-decl | array-decl | func-type-decl | enum-decl | union-decl)

        node = make_node(type_decl);

        switch(tok.key)
        {
            // struct-decl
        case op_openscope:
            printf("{ ");
            node->type_decl.data = parse_named_type_list_decl(self);
            printf(" }\n");
            break;

            // type-list-decl
        case op_openarg:
            printf("(");
            node->type_decl.data = parse_type_list_decl(self);
            printf(")");
            break;

            // array-decl
        case op_openarr:
            printf("[");
            node->type_decl.data = parse_array_decl(self);
            printf("]");
            break;

            // union-decl
        case kw_union:
            node->type_decl.data = parse_union_decl(self);
            break;

            // enum-decl
        case kw_enum:
            node->type_decl.data = parse_enum_decl(self);
            break;

            // error
        default:
            printf("unexpected type keyword\n");
            break;
        }
    }
    else
    {
        // error
    }

    keyword_t key = peek_keyword(self);
    
    // [`*`]
    node->type_decl.pointer = key == op_mul;

    return node;
}

// using-decl: `using` [ident `=` type-decl | `scope` dotted-name | `module` dotted-name]
static node_t* parse_using_decl(parser_t* self)
{
    printf("\nusing ");
    token_t tok = next_tok(self);

    node_t* node = NULL;

    if(tok.type == ident)
    {
        printf("%s ", tok.str);
        node = make_node(using_type_decl);

        // ident
        node->using_type_decl.name = tok.str;

        // `=`
        expect_keyword(self, op_assign);
        printf("= ");

        // type-decl
        node->using_type_decl.type = parse_type_decl(self);
    }
    else if(tok.type == keyword)
    {

    }
    else
    {
        // error
    }

    return node;
}

////////////////////////////////////////////////////
///             variable parsing
///////////////////////////////////////////////////

static node_t* parse_var_decl(parser_t* self)
{

}

static node_t* parse_let_decl(parser_t* self)
{

}

////////////////////////////////////////////////////
///             function parsing
////////////////////////////////////////////////////

static node_t* parse_func_decl(parser_t* self)
{

}

////////////////////////////////////////////////////
///             scope parsing
////////////////////////////////////////////////////

// scope-decl: `scope` dotted-name-decl `{` [body-decl] `}`
static node_t* parse_scope_decl(parser_t* self)
{
    printf("scope ");
    node_t* node = make_node(scope_decl);

    // scope-name-decl
    node->scope_decl.name = parse_dotted_name_decl(self);

    // [body-decl]
    node->scope_decl.decls = parse_body(self);
    return node;
}

static const key_func_pair_t body_table[] = {
    { kw_using, parse_using_decl },
    { kw_def, parse_func_decl },
    { kw_let, parse_let_decl },
    { kw_var, parse_var_decl },
    { kw_scope, parse_scope_decl }
};

static const int body_table_len = sizeof(body_table) / sizeof(key_func_pair_t);

// body-decl: [using-decl | func-decl | var-decl | val-decl | scope-decl] [body-decl]
static node_t* parse_body(parser_t* self)
{
    node_t* node = make_node(body_decl);

    // TODO: maybe this needs to be bigger
    node_stack_t nodes = { 0, malloc(sizeof(node_t*) * 128) };

    for(;;)
    {
        token_t tok = next_tok(self);

        if(tok.type == eof)
        {
            break;
        }

        if(tok.type != keyword)
        {
            // TODO: errors
            printf("\nexpected keyword got %d instead %s\n", tok.type, tok.str);
            exit(9);
        }

        for(int i = 0; i < body_table_len; i++)
        {
            if(body_table[i].key == tok.key)
            {
                // [using-decl | func-decl | var-decl | val-decl | scope-decl]
                node_stack_push(&nodes, body_table[i].func(self));

                // [body-decl]
                goto loop_again;
            }
        }

        printf("unexpected toplevel keyword %d\n", tok.key);
        exit(10);

    loop_again:continue;
    }

    node->body_decl.count = nodes.top;
    node->body_decl.decls = realloc(nodes.nodes, sizeof(node_t*) * nodes.top);

    return node;
}

node_t* parser_generate_ast(parser_t* self)
{
    return parse_body(self);
}

void name_stack_push(name_stack_t* self, char* name)
{
    self->names[self->top++] = name;
}

char* name_stack_pop(name_stack_t* self)
{
    return self->names[self->top--];
}

void node_stack_push(node_stack_t* self, node_t* node)
{
    self->nodes[self->top++] = node;
}

node_t* node_stack_pop(node_stack_t* self)
{
    return self->nodes[self->top--];
}

#endif