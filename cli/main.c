#include "common/common.h"

#include <stdio.h>

int next_file(void* f)
{
    int c = fgetc(f);
    return c;
}

int peek_file(void* f)
{
    int c = fgetc(f);
    ungetc(c, f);
    return c;
}

void close_file(void* f)
{
    fclose(f);
}

void generate_java(ctu_node* node)
{
    if(node == NULL)
    {
        return;
    }
    else if(node->type == nt_toplevel)
    {
        if(node->module_decl)
        {
            printf("package ");
            generate_java(node->module_decl->path);
            printf(";\n\n");
        }

        if(node->import_decls)
        {
            generate_java(node->import_decls);
        }

        if(node->body_decls)
        {
            generate_java(node->body_decls);
        }
    }
    else if(node->type == nt_import_decl)
    {
        for(int i = 0; i < node->count; i++)
        {
            printf("import ");
            generate_java(node->imports[i]);
            printf(";\n");
        }
    }
    else if(node->type == nt_dotted_name_decl)
    {
        printf("%s", node->parts[0]);

        for(int i = 1; i < node->count; i++)
        {
            printf(".%s", node->parts[i]);
        }
    }
    else if(node->type == nt_using_decl)
    {
        printf("class %s {\n", node->name);

        generate_java(node->typedecl);

        printf("}\n");
    }
    else if(node->type == nt_tuple_decl)
    {
        for(int i = 0; i < node->count; i++)
        {
            generate_java(node->fields[i]);
            printf(" value%d;\n", i);
        }
    }
    else if(node->type == nt_typename_decl)
    {
        generate_java(node->path);
    }
    else if(node->type == nt_struct_decl)
    {
        for(int i = 0; i < node->count; i++)
        {
            generate_java(node->fields[i]);
            printf(" %s;\n", node->names[i]);
        }
    }
}

int main(int argc, char** argv) 
{
    void* f = fopen(argv[1], "r");
    ctu_file file;
    file.data = f;
    file.next = next_file;
    file.peek = peek_file;
    file.close = close_file;

    ctu_lexer lex = lexer_alloc(file);

    ctu_parser parse = parser_alloc(lex);

    ctu_node* ast = parser_ast(&parse);

    generate_java(ast);
}

#if 0

char next_file(void* f)
{
    char c = fgetc(f);    
    return c;
}

char peek_file(void* f)
{
    char c = fgetc(f);
    ungetc(c, f);
    return c;
}

void seek_file(void* f, uint64_t pos)
{
    fseek(f, SEEK_SET, pos);
}

void close_file(void* f)
{
    fclose(f);
}

uint64_t tell_file(void* f)
{
    return ftell(f);
}

int main(int argc, char** argv)
{
    file_t file = {
        .data = (void*)fopen(argv[1], "r"),
        .next = next_file,
        .peek = peek_file,
        .close = close_file,
        .seek = seek_file,
        .tell = tell_file
    };

    lexer_t* lex = lexer_alloc(&file);

    parser_t* parse = parser_alloc(lex);

    node_t* ast = parser_generate_ast(parse);
}

#endif