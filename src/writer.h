typedef struct {
    FILE* out;
    uint64_t i;

    char buffer[128];
} Writer;

#define ASSERT(expr, msg) if(!(expr)) { printf(msg); exit(900); }
#define TEXPECT(node, t) ASSERT(node->type == t, "invalid node type")

#define WRITE(ctx, fmt, ...) fprintf(ctx->out, fmt, __VA_ARGS__)
#define PUT(ctx, fmt) fprintf(ctx->out, fmt)

#define JOIN_CHAR "_"

char* MakeName(Writer* ctx)
{
    uint64_t i;
    char* out;

    i = ctx->i++;
    out = malloc(96);
    sprintf(out, "name_%lu", i);

    return out;
}

char* IntToName(Writer* ctx, size_t i)
{
    sprintf(ctx->buffer, "_%lu", i);
    return ctx->buffer;
}

void PrintImport(Writer* ctx, Node* node)
{
    TEXPECT(node, NodeTypeImport);

    PUT(ctx, "#include \"");
    
    for(size_t i = 0; i < node->data._import.path.size; i++)
    {
        if(i != 0)
            PUT(ctx, "/");

        WRITE(ctx, "%s", node->data._import.path.arr[i]);
    }

    PUT(ctx, ".h\"");

    if(node->data._import.alias)
    {
        WRITE(ctx, " // -> %s", node->data._import.alias);
    }

    PUT(ctx, "\n");
}

char* PrintType(Writer* ctx, Node* node)
{
    char* other;
    char* name;

    vec_str_t names;

    if(node->type == NodeTypeName)
    {
        if(node->data.type._name.size == 1)
        {
            name = node->data.type._name.arr[0];
        }
        else
        {
            PUT(ctx, "typedef ");
            for(size_t i = 0; i < node->data.type._name.size; i++)
            {
                if(i != 0)
                    PUT(ctx, JOIN_CHAR);

                WRITE(ctx, "%s", node->data.type._name.arr[i]);
            }

            name = MakeName(ctx);
            WRITE(ctx, " %s;\n", name);
        }
    }
    else if(node->type == NodeTypePtr)
    {
        name = PrintType(ctx, node->data.type._ptr);
        sprintf(name, "%s*", name);
    }
    else if(node->type == NodeTypeTuple)
    {
        vec_str_init(names);

        for(size_t i = 0; i < node->data.type._tuple.size; i++)
        {
            other = PrintType(ctx, node->data.type._tuple.arr[i]);
            vec_str_append(names, other);
        }

        PUT(ctx, "typedef struct { ");

        for(size_t i = 0; i < names->size; i++)
        {
            WRITE(ctx, "%s %s; ", names->arr[i], IntToName(ctx, i));
        }

        name = MakeName(ctx);

        WRITE(ctx, "} %s;\n", name);
    }
    else if(node->type == NodeTypeStruct)
    {
        vec_str_init(names);

        for(size_t i = 0; i < node->data.type._struct.size; i++)
        {
            other = PrintType(ctx, node->data.type._struct.arr[i].node);
            vec_str_append(names, other);
        }

        PUT(ctx, "typedef struct { ");

        for(size_t i = 0; i < names->size; i++)
        {
            WRITE(ctx, "%s %s; ", names->arr[i], node->data.type._struct.arr[i].key);
        }

        name = MakeName(ctx);

        WRITE(ctx, "} %s;\n", name);
    }
    else if(node->type == NodeTypeUnion)
    {
        vec_str_init(names);
        
        for(size_t i = 0; i < node->data.type._union.size; i++)
        {
            other = PrintType(ctx, node->data.type._union.arr[i].node);
            vec_str_append(names, other);
        }

        PUT(ctx, "typedef union { ");
        
        for(size_t i = 0; i < names->size; i++)
        {
            WRITE(ctx, "%s %s; ", names->arr[i], node->data.type._union.arr[i].key);
        }

        name = MakeName(ctx);

        WRITE(ctx, "} %s;\n", name);
    }
    else if(node->type == NodeTypeEnum)
    {
        PUT(ctx, "typedef enum { ");

        for(size_t i = 0; i < node->data.type._enum.size; i++)
        {
            WRITE(ctx, "%s,", node->data.type._enum.arr[i].key);
        }

        name = MakeName(ctx);

        WRITE(ctx, "} %s;\n", name);
    }
    else if(node->type == NodeTypeVariant)
    {
        name = "void";
    }
    else
    {
        name = NULL;
    }

    return name;
}

void PrintTypedef(Writer* ctx, Node* node)
{
    char* name;
    TEXPECT(node, NodeTypeTypedef);

    name = PrintType(ctx, node->data._typedef.type);

    WRITE(ctx, "typedef %s %s;\n", name, node->data._typedef.name);
}

void PrintBody(Writer* ctx, Node* node)
{
    if(node->type == NodeTypeTypedef)
    {
        PrintTypedef(ctx, node);
    }
}

void PrintProgram(Writer* ctx, Node* node)
{
    char f[128];
    TEXPECT(node, NodeTypeProgram);

    if(node->data._program.name)
    {
        sprintf(f, "%s.h", node->data._program.name);
        ctx->out = fopen(f, "w");
        printf("writing to %s\n", f);
    }
    else
    {
        ctx->out = stdout;
    }

    for(size_t i = 0; i < node->data._program.imports.size; i++)
    {
        PrintImport(ctx, node->data._program.imports.arr[i]);
    }

    for(size_t i = 0; i < node->data._program.body.size; i++)
    {
        PrintBody(ctx, node->data._program.body.arr[i]);
    }
}
