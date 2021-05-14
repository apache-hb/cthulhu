#include "ast.h"
#include "bison.h"
#include "flex.h"
#include "scanner.h"
#include "sema.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static node_t *new_node(node_type_t type)
{
    node_t *node = malloc(sizeof(node_t));
    node->type = type;
    node->mut = false;
    node->exported = false;
    node->comptime = false;
    node->loc = NULL;
    return node;
}

static node_t *new_decl(node_type_t type, char *name)
{
    node_t *node = new_node(type);
    node->decl.name = name;
    node->decl.attribs = NULL;
    return node;
}

nodes_t *empty_list(void)
{
    nodes_t *nodes = malloc(sizeof(nodes_t));
    nodes->length = 0;
    nodes->size = 8;
    nodes->data = malloc(sizeof(node_t) * nodes->size);
    return nodes;
}

nodes_t *list(node_t *init)
{
    nodes_t *nodes = empty_list();
    return list_add(nodes, init);
}

nodes_t *list_add(nodes_t *self, node_t *node)
{
    if (self->length + 1 > self->size) {
        self->size += 8;
        self->data = realloc(self->data, self->size * sizeof(node_t));
    }
    memcpy(self->data + self->length, node, sizeof(node_t));
    self->length += 1;
    return self;
}

nodes_t *list_push(nodes_t *self, node_t *node)
{
    if (self->length + 1 > self->size) {
        self->size += 8;
        self->data = realloc(self->data, self->size * sizeof(node_t));
    }
    memmove(self->data + 1, self->data, sizeof(node_t) * self->length);
    memcpy(self->data, node, sizeof(node_t));
    self->length += 1;
    return self;
}

nodes_t *list_join(nodes_t *self, nodes_t *other)
{
    if (self->length + other->length > self->size) {
        self->size = self->length + other->length;
        self->data = realloc(self->data, self->size * sizeof(node_t));
    }
    memcpy(self->data + self->length, other->data, other->length * sizeof(node_t));
    self->length += other->length;
    return self;
}

path_t *empty_path(void) 
{
    path_t* it = malloc(sizeof(path_t));
    it->length = 0;
    it->size = 8;
    it->data = malloc(sizeof(char*) * it->size);
    return it;
}

path_t *path(char *init)
{
    path_t* it = empty_path();
    it->data[it->length++] = init;
    return it;
}

path_t *path_add(path_t *self, char *item)
{
    if (self->length + 1 > self->size) {
        self->size += 8;
        self->data = realloc(self->data, self->size * sizeof(char*));
    }
    self->data[self->length++] = item;
    return self;
}

node_t *attach(node_t *decl, nodes_t *attribs)
{
    decl->decl.attribs = attribs;
    return decl;
}

node_t *attribute(path_t *path, nodes_t *args)
{
    node_t *node = new_node(NODE_ATTRIB);
    node->attrib.path = path;
    node->attrib.args = args;
    return node;
}

node_t *comptime(node_t *decl, bool comptime)
{
    decl->comptime = comptime;
    return decl;
}

node_t *exported(node_t *decl, bool exported)
{
    decl->exported = exported;
    return decl;
}

node_t *mut(node_t *node)
{
    node->mut = true;
    return node;
}

node_t *nbreak(char *label)
{
    node_t *node = new_node(NODE_BREAK);
    node->text = label;
    return node;
}

node_t *ncontinue()
{
    return new_node(NODE_CONTINUE);
}

node_t *var(node_t *names, node_t *type, node_t *init, bool mut)
{
    node_t *node = new_decl(NODE_VAR, NULL);
    node->mut = mut;
    node->decl.var.names = names;
    node->decl.var.type = type;
    node->decl.var.init = init;
    return node;
}

node_t *func(char *name, nodes_t *params, node_t *result, node_t *body)
{
    node_t *node = new_decl(NODE_FUNC, name);
    node->decl.func.params = params;
    node->decl.func.result = result;
    node->decl.func.body = body;
    return node;
}

node_t *param(char *name, node_t *type, node_t *init)
{
    node_t *node = new_decl(NODE_PARAM, name);
    node->decl.param.type = type;
    node->decl.param.init = init;
    return node;
}

node_t *redirect(char *name)
{
    node_t *node = new_node(NODE_TYPENAME);
    node->text = name;
    return node;
}

node_t *alias(char *name, node_t *type)
{
    node_t *node = new_decl(NODE_ALIAS, name);
    node->decl.type = type;
    return node;
}

node_t *qualified(path_t *path)
{
    node_t *node = new_node(NODE_QUALIFIED);
    node->qualified = path;
    return node;
}

node_t *array(node_t *type, node_t *size)
{
    node_t *node = new_node(NODE_ARRAY);
    node->array.type = type;
    node->array.size = size;
    return node;
}

node_t *closure(nodes_t *args, node_t *result)
{
    node_t *node = new_node(NODE_CLOSURE);
    node->closure.args = args;
    node->closure.result = result;
    return node;
}

node_t *pointer(node_t *type)
{
    node_t *node = new_node(NODE_POINTER);
    node->pointer = type;
    return node;
}

node_t *builtin(char *name)
{
    node_t *node = new_decl(NODE_BUILTIN, name);
    return node;
}

node_t *record(char *name, nodes_t *items)
{
    node_t *node = new_decl(NODE_RECORD, name);
    node->decl.fields = items;
    return node;
}

node_t *branch(node_t *cond, node_t *body, node_t *next)
{
    node_t *node = new_node(NODE_BRANCH);
    node->branch.cond = cond;
    node->branch.body = body;
    node->branch.next = next;
    return node;
}

node_t *condition(node_t *init, node_t *cond)
{
    node_t *node = new_node(NODE_COND);
    node->cond.init = init;
    node->cond.cond = cond;
    return node;
}

node_t *uniondecl(char *name, nodes_t *items)
{
    node_t *node = new_decl(NODE_UNION, name);
    node->decl.fields = items;
    return node;
}

node_t *item(char *name, node_t *type)
{
    node_t *node = new_decl(NODE_ITEM, name);
    node->decl.type = type;
    return node;
}

node_t *unary(int op, node_t *expr)
{
    node_t *node = new_node(NODE_UNARY);
    node->unary.op = op;
    node->unary.expr = expr;
    return node;
}

node_t *binary(int op, node_t *lhs, node_t *rhs)
{
    node_t *node = new_node(NODE_BINARY);
    node->binary.op = op;
    node->binary.lhs = lhs;
    node->binary.rhs = rhs;
    return node;
}

node_t *ternary(node_t *cond, node_t *lhs, node_t *rhs)
{
    node_t *node = new_node(NODE_TERNARY);
    node->ternary.cond = cond;
    node->ternary.lhs = lhs;
    node->ternary.rhs = rhs;
    return node;
}

node_t *field(node_t *expr, char *field, bool indirect)
{
    node_t *node = new_node(NODE_FIELD);
    node->field.expr = expr;
    node->field.field = field;
    node->field.indirect = indirect;
    return node;
}

node_t *apply(node_t *expr, nodes_t *args)
{
    node_t *node = new_node(NODE_APPLY);
    node->apply.expr = expr;
    node->apply.args = args;
    return node;
}

node_t *include(path_t *path, nodes_t *items)
{
    node_t *node = new_node(NODE_INCLUDE);
    node->include.path = path;
    node->include.items = items;
    return node;
}

node_t *string(char *text)
{
    node_t *node = new_node(NODE_STRING);
    node->text = text;
    return node;
}

node_t *result(node_t *expr)
{
    node_t *node = new_node(NODE_RETURN);
    node->expr = expr;
    return node;
}

node_t *arg(char *name, node_t *expr)
{
    node_t *node = new_decl(NODE_ARG, name);
    node->decl.expr = expr;
    return node;
}

node_t *compound(nodes_t *stmts)
{
    node_t *node = new_node(NODE_COMPOUND);
    node->stmts = stmts;
    return node;
}

node_t *assign(node_t *lhs, node_t *rhs, int op)
{
    node_t *node = new_node(NODE_ASSIGN);
    node->assign.lhs = lhs;
    node->assign.rhs = rhs;
    node->assign.op = op;
    return node;
}

node_t *nil(void)
{
    return new_node(NODE_NULL);
}

node_t *name(char *text)
{
    node_t *node = new_node(NODE_NAME);
    node->text = text;
    return node;
}

node_t *digit(char *text, char *suffix)
{
    node_t *node = new_node(NODE_DIGIT);
    node->digit.digit = text;
    node->digit.suffix = suffix;
    return node;
}

node_t *raise(node_t *expr)
{
    node_t *node = new_node(NODE_RAISE);
    node->expr = expr;
    return node;
}

node_t *variant(char *name, node_t *type, nodes_t *cases)
{
    node_t *node = new_decl(NODE_VARIANT, name);
    node->decl.variant.type = type;
    node->decl.variant.cases = cases;
    return node;
}

node_t *variantcase(char *name, nodes_t *fields, node_t *init)
{
    node_t *node = new_decl(NODE_CASE, name);
    node->decl.vcase.fields = fields;
    node->decl.vcase.init = init;
    return node;
}

node_t *scope(path_t *path, nodes_t *decls)
{
    node_t *node = new_node(NODE_MODULE);
    node->scope.path = path;
    node->scope.decls = decls;
    return node;
}

node_t *subscript(node_t *expr, node_t *index)
{
    node_t *node = new_node(NODE_SUBSCRIPT);
    node->subscript.expr = expr;
    node->subscript.index = index;
    return node;
}

node_t *cast(node_t *expr, node_t *type)
{
    node_t *node = new_node(NODE_CAST);
    node->cast.expr = expr;
    node->cast.type = type;
    return node;
}

node_t *nwhile(char *label, node_t *cond, node_t *body, node_t *tail)
{
    node_t *node = new_decl(NODE_WHILE, label);
    node->decl.loop.cond = cond;
    node->decl.loop.body = body;
    node->decl.loop.tail = tail;
    return node;
}

node_t *items(nodes_t *items)
{
    node_t *node = new_node(NODE_LIST);
    node->items = items;
    return node;
}

node_t *nfor(char *label, node_t *names, node_t *range, node_t *body, node_t *tail)
{
    node_t *node = new_decl(NODE_FOR, label);
    node->decl.iter.names = names;
    node->decl.iter.range = range;
    node->decl.iter.body = body;
    node->decl.iter.tail = tail;
    return node;
}

node_t *with(node_t *init, node_t *body)
{
    node_t *node = new_node(NODE_WITH);
    node->with.init = init;
    node->with.body = body;
    return node;
}

static int depth = 0;

static void ln(int change)
{
    depth += change;
    printf("\n");
    for (int i = 0 ; i < depth * 2; i++) 
        printf(" ");
}

void dump_nodes(nodes_t *nodes, bool feed)
{
    for (size_t i = 0; i < nodes->length; i++) {
        if (i != 0) {
            if (feed)
                ln(0);
            else    
                printf(" ");
        }
        dump_node(nodes->data + i);
    }
}

static void dump_path(path_t *path)
{
    for (size_t i = 0; i < path->length; i++) {
        if (i != 0) {
            printf(" ");
        }
        printf("%s", path->data[i]);
    }
}

void dump_node(node_t *node)
{    
    node_t *tail = node;

    bool set = false;
    bool exported = node->exported;
    bool mut = node->type > NODE_MINTYPE && node->type < NODE_MAXTYPE;
    bool decl = node->type > NODE_MINDECL && node->type < NODE_MAXDECL;
    bool attribs = decl && node->decl.attribs && node->decl.attribs->length > 0;
    bool comptime = node->comptime;

    if (exported) {
        printf("(export ");
    }

    if (mut) {
        printf(node->mut ? "(mut " : "(final ");
    }

    if (comptime) {
        printf("(compile ");
    }

    if (attribs) {
        printf("(attribs %zu", node->decl.attribs->length);
        ln(1); dump_nodes(node->decl.attribs, true);
        ln(0); printf("decl = ");
    }

    switch (node->type) {
    case NODE_DIGIT:
        printf("(digit `%s`", node->digit.digit);
        if (node->digit.suffix) {
            printf(" suffix = `%s`", node->digit.suffix);
        }
        printf(")");
        break;
    case NODE_STRING:
        printf("(string %s)", node->text);
        break;
    case NODE_NULL:
        printf("nil");
        break;
    case NODE_UNARY:
        switch (node->unary.op) {
        case ADD: printf("(abs "); break;
        case SUB: printf("(neg "); break;
        case BITAND: printf("(ref "); break;
        case MUL: printf("(deref "); break;
        case NOT: printf("(not "); break;
        case FLIP: printf("(flip "); break;
        default: printf("(err"); break;
        }
        dump_node(node->unary.expr);
        printf(")");
        break;
    case NODE_BINARY:
        switch (node->binary.op) {
        case ADD: printf("(add "); break;
        case SUB: printf("(sub "); break;
        case DIV: printf("(div "); break;
        case MUL: printf("(mul "); break;
        case REM: printf("(rem "); break;
        case GT: printf("(gt "); break;
        case GTE: printf("(gte "); break;
        case LT: printf("(lt "); break;
        case LTE: printf("(lte "); break;
        case NEQ: printf("(neq "); break;
        case EQ: printf("(eq "); break;
        case SHL: printf("(shl "); break;
        case SHR: printf("(shr "); break;
        case BITAND: printf("(bitand "); break;
        case BITOR: printf("(bitor "); break;
        case BITXOR: printf("(bitxor "); break;
        case AND: printf("(and "); break;
        case OR: printf("(or "); break;
        default: printf("(err"); break;
        }
        dump_node(node->binary.lhs);
        printf(" ");
        dump_node(node->binary.rhs);
        printf(")");
        break;
    case NODE_TERNARY:
        if (node->ternary.lhs) {
            printf("(if "); dump_node(node->ternary.cond); ln(1);
            printf("then "); dump_node(node->ternary.lhs); ln(0);
            printf("else "); dump_node(node->ternary.rhs); ln(-1);
            printf(")");
        } else {
            printf("(unwrap "); dump_node(node->ternary.cond); ln(1);
            printf("else "); dump_node(node->ternary.rhs); ln(-1);
            printf(")");
        }
        break;
    case NODE_FIELD:
        printf("(access "); dump_node(node->field.expr); ln(1);
        printf("field `%s`", node->field.field); ln(-1);
        printf(")");
        break;
    case NODE_APPLY:
        printf("(apply "); depth++;
        dump_node(node->apply.expr); ln(0);
        dump_nodes(node->apply.args, true); ln(-1);
        printf(")");
        break;
    case NODE_CAST:
        printf("(cast "); dump_node(node->cast.expr); 
        printf(" "); dump_node(node->cast.type);
        printf(")");
        break;
    case NODE_SUBSCRIPT:
        printf("(index "); dump_node(node->subscript.expr);
        printf(" "); dump_node(node->subscript.index);
        printf(")");
        break;
    case NODE_TYPENAME:
        printf("(typename `%s`)", node->text);
        break;
    case NODE_VAR:
        printf("(%s ", node->mut ? "var" : "final"); 
        
        printf("names = "); dump_node(node->decl.var.names);

        if (node->decl.var.type) {
            ln(1);
            set = true;

            printf("type = "); dump_node(node->decl.var.type); 
        }

        if (node->decl.var.init) {
            ln(set ? 0 : 1);
            set = true;
            
            printf("init = "); dump_node(node->decl.var.init);
        }

        if (set) {
            ln(-1);
        }

        printf(")");
        break;
    case NODE_FUNC:
        if (!node->decl.func.body) {
            printf("(import `%s`", node->decl.name);
        } else {
            printf("(define `%s`", node->decl.name);
        }
        
        ln(1); printf("args = (");
        dump_nodes(node->decl.func.params, false);
        printf(")");

        ln(0); printf("result = ");
        if (node->decl.func.result) {
            dump_node(node->decl.func.result);
        } else {
            printf("empty");
        }

        if (node->decl.func.body) {
            ln(0); printf("body = "); dump_node(node->decl.func.body);
        }

        ln(-1); printf(")");
        break;
    case NODE_PARAM:
        printf("(param `%s` ", node->decl.name);
        printf("type = "); dump_node(node->decl.param.type);
        if (node->decl.param.init) {
            printf("init = "); dump_node(node->decl.param.init);
        }
        printf(")");
        break;
    case NODE_BUILTIN:
        printf("(builtin `%s`)", node->decl.name);
        break;
    case NODE_POINTER:
        printf("(ptr ");
        dump_node(node->pointer);
        printf(")");
        break;
    case NODE_ARRAY:
        printf("(array");
        ln(1); printf("type = "); dump_node(node->array.type);
        if (node->array.size) {
            ln(0); printf("size = "); dump_node(node->array.size);
        }
        ln(-1); printf(")");
        break;
    case NODE_CLOSURE:
        printf("(closure"); 
        ln(1); printf("result = "); dump_node(node->closure.result);
        ln(0); printf("args = ("); dump_nodes(node->closure.args, false); printf(")");
        ln(-1); printf(")");
        break;
    case NODE_QUALIFIED:
        printf("(qualified ");
        dump_path(node->qualified);
        printf(")");
        break;
    case NODE_ALIAS:
        printf("(alias `%s` ", node->decl.name);
        dump_node(node->decl.type);
        printf(")");
        break;
    case NODE_ITEM:
        printf("(item `%s` ", node->decl.name);
        dump_node(node->decl.type);
        printf(")");
        break;
    case NODE_RECORD:
        printf("(record `%s` fields = (", node->decl.name);
        ln(1); dump_nodes(node->decl.fields, true);
        ln(-1); printf("))");
        break;
    case NODE_UNION:
        printf("(union `%s` fields = (", node->decl.name);
        ln(1); dump_nodes(node->decl.fields, true);
        ln(-1); printf("))");
        break;
    case NODE_INCLUDE:
        printf("(include path = (");
        dump_path(node->include.path);
        printf(")");
        if (node->include.items) {
            ln(1); printf("items = (");
            if (node->include.items->length == 0) {
                printf("*");
            } else {
                dump_nodes(node->include.items, false);
            }
            printf(")");
            ln(-1);
        }
        printf(")");
        break;
    case NODE_VARIANT:
        printf("(variant `%s`", node->decl.name);
        if (node->decl.variant.type) {
            ln(1); printf("type = "); dump_node(node->decl.variant.type);
            set = true;
        }

        if (!set) {
            ln(1);
        } else {
            ln(0);
        }

        printf("cases = (");
        ln(1); dump_nodes(node->decl.variant.cases, true);
        ln(-1); printf(")");

        ln(-1); printf(")");
        break;
    case NODE_CASE:
        printf("(case `%s`", node->decl.name);
        if (node->decl.vcase.init) {
            ln(1); printf("init = "); dump_node(node->decl.vcase.init);
        }
        
        if (node->decl.vcase.fields) {
            if (node->decl.vcase.init) {
                ln(0);
            } else {
                ln(1);
            }

            printf("fields = (");
            ln(1); dump_nodes(node->decl.vcase.fields, true);
            ln(-1); printf(")");
        }

        ln(-1); printf(")");
        break;
    case NODE_MODULE:
        printf("(module "); dump_path(node->scope.path);
        ln(1); printf("decls = (");
        ln(1); dump_nodes(node->scope.decls, true);
        ln(-1); printf(")");
        ln(-1); printf(")");
        break;

    case NODE_RETURN:
        if (node->expr) {
            printf("(return "); dump_node(node->expr); printf(")");
        } else {
            printf("return");
        }
        break;
    case NODE_COMPOUND:
        printf("(compound");
        if (node->stmts->length) {
            ln(1); dump_nodes(node->stmts, true);
            ln(-1); 
        }
        printf(")");
        break;
    case NODE_ASSIGN:
        switch (node->assign.op) {
        case ASSIGN: printf("(assign "); break;
        case ADDEQ: printf("(addeq "); break;
        case SUBEQ: printf("(subeq "); break;
        case DIVEQ: printf("(diveq "); break;
        case MULEQ: printf("(muleq "); break;
        case REMEQ: printf("(remeq "); break;
        case ANDEQ: printf("(andeq "); break;
        case OREQ: printf("(oreq "); break;
        case XOREQ: printf("(xoreq "); break;
        case SHLEQ: printf("(shleq "); break;
        case SHREQ: printf("(shreq "); break;
        default: printf("(err "); break;
        }
        dump_node(node->assign.lhs);
        printf(" from ");
        dump_node(node->assign.rhs);
        printf(")");
        break;
    case NODE_BREAK:
        if (node->text) {
            printf("(break `%s`)", node->text);
        } else {
            printf("break");
        }
        break;
    case NODE_CONTINUE:
        printf("continue");
        break;
    case NODE_ARG:
        if (node->decl.name) {
            printf("(arg `%s` ", node->decl.name);
        } else {
            printf("(arg ");
        }
        dump_node(node->decl.expr);
        printf(")");
        break;
    case NODE_ATTRIB:
        printf("(attrib "); dump_path(node->attrib.path);
        if (node->attrib.args) {
            ln(1); printf("args = (");
            dump_nodes(node->attrib.args, false);
            ln(-1); printf(")");
        }
        printf(")");
        break;
    case NODE_WHILE:
        printf("(while ");
        if (node->decl.name) {
            printf("label = `%s`", node->decl.name);
        }
        ln(1); printf("cond = "); dump_node(node->decl.loop.cond);
        ln(0); printf("body = "); dump_node(node->decl.loop.body);
        if (node->decl.loop.tail) {
            ln(0); printf("else = "); dump_node(node->decl.loop.tail);
        }
        ln(-1); printf(")");
        break;
    case NODE_FOR:
        printf("(for ");
        if (node->decl.name) {
            printf("label = `%s`", node->decl.name);
        }
        ln(1); printf("names = "); dump_node(node->decl.iter.names); 
        ln(0); printf("range = "); dump_node(node->decl.iter.range);
        ln(0); printf("body = "); dump_node(node->decl.iter.body);
        if (node->decl.iter.tail) {
            printf("else = "); dump_node(node->decl.iter.tail);
        }
        ln(-1); printf(")");
        break;
    case NODE_NAME:
        printf("`%s`", node->text);
        break;
    case NODE_LIST:
        printf("("); dump_nodes(node->items, false); printf(")");
        break;
    case NODE_COND:
        printf("(cond "); 
        dump_node(node->cond.init);
        printf(" ");
        dump_node(node->cond.cond);
        printf(")");
        break;
    case NODE_BRANCH:
        printf("(branch");
        ln(1);
        while (tail) {
            if (tail->branch.cond) {
                printf("(if "); dump_node(tail->branch.cond);
            } else {
                printf("(else ");
            }
            ln(1); printf("then = "); 
            dump_node(tail->branch.body);
            ln(-1); printf(")");
            tail = tail->branch.next;
            if (tail) {
                ln(0);
            }
        }
        ln(-1); printf(")");
        break;
    case NODE_RAISE:
        printf("(raise ");
        dump_node(node->expr);
        printf(")");
        break;
    case NODE_WITH:
        printf("(with ");
        ln(1); printf("init = "); dump_node(node->with.init);
        ln(0); printf("body = "); dump_node(node->with.body);
        ln(-1); printf(")");
        break;

    case NODE_MINTYPE: case NODE_MAXTYPE:
    case NODE_MINDECL: case NODE_MAXDECL:
        printf("(invalid %d)", node->type);
        break;
    }

    if (attribs) {
        ln(-1); printf(")");
    }

    if (comptime) {
        printf(")");
    }

    if (mut) {
        printf(")");
    }

    if (exported) {
        printf(")");
    }
}

node_t *compile_common(path_t *mod, const char *path, FILE *file)
{
    int err;
    yyscan_t scan;
    scan_extra_t extra = { path, mod, NULL };

    if ((err = yylex_init_extra(&extra, &scan))) {
        ERRF("ICE: yylex_init = %d", err);
        return NULL;
    }

    yyset_in(file, scan);

    if ((err = yyparse(scan, &extra))) {
        return NULL;
    }

    yylex_destroy(scan);

    return extra.ast;
}

node_t *compile_file(char *name, const char *fs, FILE *file)
{
    return compile_common(path(name), fs, file);
}

static char *join_path_with(path_t *path, char sep, const char *ext)
{
    size_t needed = strlen(ext) + path->length;

    for (size_t i = 0; i < path->length; i++) {
        needed += strlen(path->data[i]);
    }

    char *out = malloc(needed + 1);

    char *cur = out;
    for (size_t i = 0; i < path->length; i++) {
        if (i != 0) {
            *cur++ = sep;
        }

        char *part = path->data[i];
        size_t len = strlen(part);
        strcpy(cur, part);
        cur += len;
    }

    strcpy(cur, ext);
    return out;
}

node_t *compile_by_path(path_t *path)
{
    char *search = join_path_with(path, '/', ".ct");

    FILE *source = fopen(search, "r");

    if (!source) {
        REPORTF("failed to open file %s", search);
        return NULL;
    }

    return compile_common(path, search, source);
}

char *path_at(path_t *path, size_t idx)
{
    return path->data[idx];
}

char *path_tail(path_t *path)
{
    return path_at(path, path->length - 1);
}

int yyerror(YYLTYPE *yylloc, void *scanner, scan_extra_t *extra, const char *msg) {
    (void)scanner;

    ERRF("[%s:%d:%d]: %s", 
        extra->path,
        yylloc->first_line, 
        yylloc->first_column, 
        msg
    );
    return 1;
}
