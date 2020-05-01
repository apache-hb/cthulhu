typedef struct {
    FILE* out;
} Writer;

#define ASSERT(expr, msg) if(!(expr)) { printf(msg); exit(900); }
#define TEXPECT(node, t) ASSERT(node->type == t, "invalid node type")

#define WRITE(ctx, fmt, ...) fprintf(ctx->out, fmt, __VA_ARGS__)
#define PUT(ctx, fmt) fprintf(ctx->out, fmt)

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
        WRITE(ctx, " // as %s", node->data._import.alias);
    }

    PUT(ctx, "\n");
}

void PrintType(Writer* ctx, Node* node)
{
    (void)ctx;
    (void)node;

    if(node->type == NodeTypeStruct)
    {
        
    }
    else if(node->type == NodeTypeTuple)
    {

    }
    else if(node->type == NodeTypeArray)
    {

    }
    else if(node->type == NodeTypeEnum)
    {

    }
    else if(node->type == NodeTypeVariant)
    {

    }
    else if(node->type == NodeTypeName)
    {

    }
    else if(node->type == NodeTypeUnion)
    {

    }
}

void PrintTypedef(Writer* ctx, Node* node)
{
    TEXPECT(node, NodeTypeTypedef);

    PrintType(ctx, node->data._typedef.type);
}

void PrintBody(Writer* ctx, Node* node)
{
    if(!node)
        PUT(ctx, "oh no");
        
    if(node->type == NodeTypeTypedef)
    {
        PrintTypedef(ctx, node);
    }
}

void PrintProgram(Writer* ctx, Node* node)
{
    TEXPECT(node, NodeTypeProgram);

    if(node->data._program.name)
    {
        ctx->out = fopen(node->data._program.name, "w");
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