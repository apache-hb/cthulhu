#include "sema.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int errors = 0;

static modules_t *modules;

static modules_t *module_list(void)
{
    modules_t* mods = malloc(sizeof(modules_t));
    mods->length = 0;
    mods->size = 4;
    mods->data = malloc(sizeof(module_t) * mods->size);
    return mods;
}

static void modules_add(modules_t *self, module_t *mod)
{
    if (self->length + 1 >= self->size) {
        self->size += 4;
        self->data = realloc(self->data, sizeof(module_t) * self->size);
    }
    memcpy(self->data + self->length, mod, sizeof(module_t));
    self->length += 1;
}

static module_t *module_at(modules_t *self, size_t idx)
{
    return self->data + idx;
}

static module_t *new_module(module_t *parent, const char *name)
{
    module_t *mod = malloc(sizeof(module_t));
    mod->name = name;
    mod->parent = parent;
    mod->children = module_list();
    mod->decls = empty_list();

    if (parent) {
        modules_add(parent->children, mod);
    }

    return mod;
}

static module_t *root_module(module_t *mod)
{
    return mod->parent ? root_module(mod->parent) : mod;
}

static node_t *node_at(nodes_t *nodes, size_t idx)
{
    return nodes->data + idx;
}

static const char *get_decl_name(node_t *node)
{
    switch (node->type) {
    case NODE_FUNC: case NODE_ALIAS:
        return node->decl.name;

    default:
        REPORTF("get_decl_name %d", node->type);
        return NULL;
    }
}

static node_t *find_decl(module_t *ctx, const char *name, bool recurse)
{
    for (size_t i = 0; i < ctx->decls->length; i++) {
        node_t *node = node_at(ctx->decls, i);
        
        if (strcmp(get_decl_name(node), name) == 0) {
            return node;
        }
    }

    if (recurse && ctx->parent) {
        return find_decl(ctx->parent, name, recurse);
    }

    return NULL;
}

static node_t *find_local_decl(module_t *ctx, const char *name)
{
    return find_decl(ctx, name, false);
}

static void add_alias_decl(module_t *ctx, node_t *decl)
{
    const char *name = decl->decl.name;

    if (find_local_decl(ctx, name)) {
        REPORTF("alias `%s` clashes with existing decl", name);
    }

    list_add(ctx->decls, decl);
}


static module_t *compile_ast(path_t *path);

static void add_import_decl(module_t *ctx, node_t *decl)
{
    const char *name = path_tail(decl->include.path);

    if (find_local_decl(ctx, name)) {
        REPORTF("import clashes with decl `%s`", name);
    } else {
        compile_ast(decl->include.path);
    }
}

static module_t *find_module(module_t *ctx, const char *name)
{
    for (size_t i = 0; i < ctx->children->length; i++) {
        module_t *mod = module_at(ctx->children, i);

        if (strcmp(mod->name, name) == 0) {
            return mod;
        }
    }

    return new_module(ctx, name);
}

static void add_module_decl(module_t *ctx, node_t *decl)
{    
    module_t *root = ctx;

    for (size_t i = 0; i < decl->scope.path->length; i++) {
        const char *name = path_at(decl->scope.path, i);
        root = find_module(root, name);
    }
}

static void add_decl(module_t *ctx, node_t *decl)
{
    switch (decl->type) {
    case NODE_INCLUDE:
        add_import_decl(ctx, decl);
        break;
    case NODE_MODULE:
        add_module_decl(ctx, decl);
        break;
    case NODE_ALIAS:
        add_alias_decl(ctx, decl);
        break;

    default:
        printf("add_decl %d\n", decl->type);
    }
}

static void resolve(module_t *root, node_t *ast)
{
    for (size_t i = 0; i < ast->scope.decls->length; i++) {
        node_t *decl = node_at(ast->scope.decls, i);
        add_decl(root, decl);
    }
}

static module_t *verify_ast(module_t *parent, node_t *node)
{
    module_t *mod = parent;
    
    for (size_t i = 0; i < node->scope.path->length; i++) {
        mod = new_module(mod, path_at(node->scope.path, i));
    }

    resolve(mod, node);

    return mod;
}

static module_t *compile_ast(path_t *path)
{
    node_t *ast = compile_by_path(path);
    module_t *mod = verify_ast(NULL, ast);
    modules_add(modules, root_module(mod));
    return mod;
}

modules_t *compile_program(const char *start, FILE *source)
{
    modules = module_list();

    node_t *ast = compile_file(strdup("main"), start, source);
    module_t *mod = new_module(NULL, "main");
    mod = verify_ast(mod, ast);
    modules_add(modules, mod);

    return modules;
}

static int depth = 0;

static void ln(int change)
{
    depth += change;
    printf("\n");
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
}

static const char *node_typename(node_t *node)
{
    switch (node->type) {
    case NODE_ALIAS: return "alias";
    case NODE_FUNC: return "func";
    case NODE_VAR: return "var";
    case NODE_MODULE: return "module";

    default:
        return "unknown";
    }
}

void dump_module(module_t *mod)
{
    printf("- module: `%s`", mod->name);
    ln(1); printf("children: `%zu`", mod->children->length);
    ln(0); printf("decls: `%zu`", mod->decls->length);
    
    if (mod->decls->length) {
        ln(1);
        for (size_t i = 0; i < mod->decls->length; i++) {
            if (i) {
                ln(0);
            }
            node_t *node = node_at(mod->decls, i);
            printf("%s %s", node_typename(node), node->decl.name);
        }
        ln(-1);
    }

    if (mod->children->length) {
        ln(0);
        for (size_t i = 0; i < mod->children->length; i++) {
            if (i) {
                ln(0);
            }
            module_t *node = module_at(mod->children, i);
            dump_module(node);
        }
    }

    ln(-1);
}
