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

    PUT(ctx, "import");
    
    for(size_t i = 0; i < node->data._import.path.size; i++)
    {
        if(i != 0)
            PUT(ctx, ":");

        WRITE(ctx, "%s", node->data._import.path.arr[i]);
    }

    if(node->data._import.alias)
    {
        WRITE(ctx, " -> %s", node->data._import.alias);
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
}