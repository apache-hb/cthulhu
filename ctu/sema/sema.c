#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/util.h"
#include "ctu/util/str.h"

#include "ctu/ast/compile.h"

#include "ctu/debug/type.h"
#include "ctu/debug/ast.h"

#include <errno.h>

typedef struct sema_t {
    struct sema_t *parent;

    /** 
     * @var map_t<string, sema_t*> 
     * 
     * map of imports to their respective scope
     */
    map_t *imports;

    /** 
     * @var map_t<string, node_t*> 
     * 
     * variables in the current scope
     */
    map_t *vars;

    /** 
     * @var map_t<string, node_t*> 
     * 
     * type declarations
     */
    map_t *types;

    /** 
     * @var map_t<string, node_t*> 
     * 
     * function declarations
     */
    map_t *funcs;


    /**
     * the return type of the current function
     */
    type_t *result;

    /**
     * counters for certain types of nodes
     */

    /* the local index for the current function */
    size_t locals;
} sema_t;

/**
 * builtin types
 */
static map_t *builtins = NULL;

/**
 * all parsed files
 * kept in a map so we never open a file twice
 */
static map_t *files = NULL;

/**
 * total number of strings
 */
static size_t strings = 0;

static size_t closures = 0;

static size_t locals = 0;

static list_t *current = NULL;

static list_t *funcs = NULL;
static list_t *vars = NULL;
static list_t *types = NULL;

static list_t *headers = NULL;
static list_t *libs = NULL;

static bool loop = false;

static sema_t *begin_file(node_t *inc, node_t *root, const char *path);

static sema_t *new_sema(sema_t *parent) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));

    if (parent) {
        sema->imports = parent->imports;
        sema->result = parent->result;
    } else {
        sema->imports = new_map(8);
        sema->result = NULL;
    }

    sema->parent = parent;
    sema->vars = new_map(8);
    sema->types = new_map(8);
    sema->funcs = new_map(8);
    sema->locals = 0;

    return sema;
}

static void mark_local(node_t *node) {
    node->local = locals++;
}

static char *path_of(list_t *it) {
    char *joined = str_join("/", (const char**)it->data, it->len);
    char *out = format("%s.ct", joined);
    ctu_free(joined);

    return out;
}

static void add_import(sema_t *sema, node_t *it) {
    /**
     * compile the imported file
     */
    char *path = path_of(it->path);
    sema_t *other = begin_file(it, NULL, path);

    /**
     * then add it to the current scope
     * currently we add it by taking the last element
     * of the path and using that as the key.
     */
    map_put(sema->imports, list_last(it->path), other);
}

static void put_unique(map_t *dst, const char *key, node_t *node) {
    if (map_get(dst, key)) {
        reportf(LEVEL_ERROR, node, "redeclaration of `%s`", key);
    } else {
        map_put(dst, key, node);
    }
}

static void put_decl_unique(sema_t *sema, node_t *node) {
    const char *key = get_decl_name(node);
    if (map_get(sema->vars, key) != NULL || map_get(sema->funcs, key) != NULL) {
        reportf(LEVEL_ERROR, node, "redeclaration of `%s`", key);
    } else {
        switch (node->kind) {
        case AST_DECL_VAR: 
            map_put(sema->vars, key, node);
            break;
        case AST_DECL_FUNC:
            map_put(sema->funcs, key, node);
            break;

        default:
            assert("put_decl_unique invalid node %d", node->kind);
            break;
        }
    }
}

static void add_decl(sema_t *sema, node_t *it) {
    const char *name = get_decl_name(it);

    it->ctx = sema;

    switch (it->kind) {
    case AST_DECL_VAR: case AST_DECL_FUNC:
        put_decl_unique(sema, it);
        break;

    case AST_DECL_STRUCT:
        connect_type(it, new_struct(it, name));
        put_unique(sema->types, name, it);
        break;

    case AST_DECL_UNION:
        connect_type(it, new_union(it, name));
        put_unique(sema->types, name, it);
        break;

    case AST_DECL_ENUM:
        connect_type(it, new_enum(it, name));
        put_unique(sema->types, name, it);
        break;

    default:
        assert("unknown add-decl kind %d", it->kind);
        break;
    }
}

static void add_discard(map_t *dst, node_t *it) {
    const char *name = get_decl_name(it);
    if (!is_discard_name(name)) {
        map_put(dst, name, it);
    }
}

static void build_type(node_t *it);
static void build_var(sema_t *sema, node_t *it);
static void build_func(node_t *it);
static type_t *query_expr(sema_t *sema, node_t *it);

#include "type.c"
#include "expr.c"
#include "stmt.c"

static void add_header(char *str) {
    if (!str) {
        return;
    }
    
    for (size_t i = 0; i < list_len(headers); i++) {
        if (strcmp(str, list_at(headers, i)) == 0) {
            return;
        }
    }
    list_push(headers, str);
}

static void add_lib(char *str) {
    if (!str) {
        return;
    }

    for (size_t i = 0; i < list_len(libs); i++) {
        if (strcmp(str, list_at(libs, i)) == 0) {
            return;
        }
    }
    list_push(libs, str);
}

static char *get_str(node_t *node) {
    if (node->kind != AST_ARG) {
        reportf(LEVEL_ERROR, node, "decorator argument requires string argument");
        return NULL;
    }

    node_t *arg = node->arg;

    if (arg->kind != AST_STRING) {
        reportf(LEVEL_ERROR, node, "decorator requires string");
        return NULL;
    }

    return arg->string;
}

static void add_interop(node_t *decl, list_t *args) {
    /* first arg is header, then library */
    /* TODO: once a builtin system is in place, use that instead */

    if (list_len(args) > 0) {
        char *header = get_str(list_at(args, 0));
        add_header(header);
    }

    if (list_len(args) > 1) {
        char *library = get_str(list_at(args, 1));
        add_lib(library);
    }

    mark_interop(decl);
}

static void add_section(node_t *decl, list_t *args) {
    if (list_len(args) == 0) {
        reportf(LEVEL_ERROR, decl, "section requires name");
    } else {
        char *name = get_str(list_at(args, 0));
        mark_section(decl, name);
    }
}

typedef struct {
    const char *name;
    void(*process)(node_t*, list_t*);
} attr_t;

attr_t attrs[] = {
    { "interop", add_interop },
    { "section", add_section }
};

static bool apply_attrib(node_t *attrib, node_t *decl) {
    list_t *name = attrib->attr;

    /* for now all atributes have a length of 1 */
    if (list_len(name) != 1) {
        return false;
    }

    const char *attr = list_at(name, 0);
    for (size_t i = 0; i < sizeof(attrs) / sizeof(attr_t); i++) {
        if (strcmp(attr, attrs[i].name) == 0) {
            attrs[i].process(decl, attrib->args);
            return true;
        }
    }

    return false;
}

static void check_attribs(node_t *decl) {
    ASSERT(
        decl->kind == AST_DECL_FUNC || 
        decl->kind == AST_DECL_VAR || 
        decl->kind == AST_DECL_STRUCT ||
        decl->kind == AST_DECL_UNION ||
        decl->kind == AST_DECL_ENUM
    )("node cannot have attributes");

    list_t *decorate = decl->decorate;
    for (size_t i = 0; i < list_len(decorate); i++) {
        node_t *attr = list_at(decorate, i);

        if (!apply_attrib(attr, decl)) {
            reportf(LEVEL_WARNING, attr, "unknown attribute");
        }
    }
}

static void build_type(node_t *it) {
    list_push(current, it);
    sema_t *sema = it->ctx;
    switch (it->kind) {
    case AST_DECL_STRUCT: case AST_DECL_UNION:
        build_record(sema, it);
        break;

    default:
        assert("unknown type node %d", it->kind);
        goto clear;
    }

    check_attribs(it);

    if (is_interop(it)) {
        get_resolved_type(it)->interop = true;
    }

clear:
    list_pop(current);
}

static void build_var(sema_t *sema, node_t *it) {
    ASSERT(it->init || it->type)("var must be partialy initialized");

    check_attribs(it);

    list_push(current, it);

    type_t *type = NULL;
    type_t *init = NULL;

    type_t *out = NULL;

    if (it->init) {
        init = query_expr(sema, it->init);
        out = init;
    }

    if (it->type) {
        type = query_type(sema, it->type);
        out = type;
    }

    /**
     * variables are lvalues
     */

    out = set_lvalue(out, true);
    out = set_mut(out, is_mut(it));

    if (type != NULL && init != NULL) {
        if (!type_can_become_implicit(it->init, type, init)) {
            reportid_t id = reportf(LEVEL_ERROR, it, "incompatible types for initialization of variable `%s`", it->name);
            report_underline(id, format("found: %s", typefmt(init)));
            report_note(id, format("required %s", typefmt(type)));
        }
    }

    if (type == NULL && is_array(init)) {
        if (init->of == NULL) {
            reportid_t id = reportf(LEVEL_ERROR, it->init, "array initialization requires type");
            report_note(id, format("empty array without `of` specifier does not convey enough type information"));
        }
    }

    if (type && is_array(type)) {
        if (!init && type->unbounded) {
            reportf(LEVEL_ERROR, it, "unbounded array must be initialized");
        }
    }

    it->locals = locals;

    connect_type(it, out);
    list_pop(current);
}

static void build_global_var(node_t *it) {
    build_var(it->ctx, it);
}

static void build_params(sema_t *sema, list_t *params) {
    for (size_t i = 0; i < list_len(params); i++) {
        node_t *param = list_at(params, i);
        type_t *type = query_type(sema, param);

        add_discard(sema->vars, param);
        mark_local(param);

        if (is_void(type)) {
            reportf(LEVEL_ERROR, param, "void type not allowed as parameter");
        }

        connect_type(param, type);
    }
}

static void build_func(node_t *it) {
    sema_t *sema = it->ctx;
    sema_t *nest = new_sema(sema);
    build_params(nest, it->params);

    nest->result = query_type(sema, it->result);

    check_attribs(it);

    if (is_interop(it) && it->body) {
        reportf(LEVEL_ERROR, it, "interop function `%s` must be stubbed", it->name);
    }

    if (it->body) {
        build_stmts(nest, it->body);
    }
    
    it->locals = locals;
    locals = 0;
}

/**
 * `inc` is the import that caused this file to be parsed
 * parse a file at `path`
 * if root is null then this function will `ctu_open` 
 * the path and attempt to parse it. otherwise it will use it
 */
static sema_t *begin_file(node_t *inc, node_t *root, const char *path) {
    /**
     * if the files already parsed
     * then reuse it
     */
    sema_t *sema = map_get(files, path);
    if (sema != NULL) {
        return sema;
    }
    
    /**
     * if we havent been given a root node
     * then open the file and parse it
     */
    if (root == NULL) {
        FILE *fd = ctu_open(path, "rb");
        if (!fd) {
            reportf(LEVEL_ERROR, inc, "failed to open `%s` due to `%s`", path, strerror(errno));
            return NULL;
        }

        root = compile_file(path, fd, NULL);
        if (root == NULL) {
            return NULL;
        }
    }

    /**
     * now typecheck the ast
     */
    sema = new_sema(NULL);
    map_put(files, path, sema);

    /**
     * first add all its imports
     */
    list_t *imports = root->imports;
    for (size_t i = 0; i < list_len(imports); i++) {
        add_import(sema, list_at(imports, i));
    }

    list_t *decls = root->decls;

    /**
     * we add all the decls first for order independant
     * lookup
     */

    for (size_t i = 0; i < list_len(decls); i++) {
        node_t *decl = list_at(decls, i);
        add_decl(sema, decl);
    }

    return sema;
}

static void build_type_wrap(const char *id, void *data, void *arg) {
    (void)id;
    (void)arg;
    build_type(data);
    list_push(types, data);
}

static void build_var_wrap(const char *id, void *data, void *arg) {
    (void)id;
    (void)arg;
    build_global_var(data);
    list_push(vars, data);
}

static void build_func_wrap(const char *id, void *data, void *arg) {
    (void)id;
    (void)arg;
    build_func(data);
    list_push(funcs, data);
}

static void build_file(sema_t *sema) {
    map_iter(sema->types, build_type_wrap, sema);
    map_iter(sema->vars, build_var_wrap, sema);
    map_iter(sema->funcs, build_func_wrap, sema);
}

static void build_file_wrap(const char *id, void *data, void *arg) {
    (void)arg;
    (void)id;
    build_file(data);
}

unit_t typecheck(node_t *root) {
    funcs = new_list(NULL);
    vars = new_list(NULL);
    types = new_list(NULL);

    headers = new_list(NULL);
    libs = new_list(NULL);

    current = new_list(NULL);

    begin_file(NULL, root, root->scanner->path);

    map_iter(files, build_file_wrap, NULL);

    unit_t unit = { 
        funcs, vars, types, 
        libs, headers,
        strings, closures
    };

    return unit;
}

static void add_builtin(type_t *type) {
    map_put(builtins, type->node->name, type);
}

void sema_init(void) {
    files = new_map(8);
    builtins = new_map(32);

    for (int i = 0; i < INTEGER_END; i++) {
        add_builtin(get_int_type(false, i));
        add_builtin(get_int_type(true, i));
    }

    add_builtin(STRING_TYPE);
    add_builtin(BOOL_TYPE);
    add_builtin(VOID_TYPE);
}
