// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/sema/attrib.h"

#include "arena/arena.h"
#include "base/util.h"
#include "cthulhu/events/events.h"
#include "ctu/ast.h"

#include "cthulhu/tree/query.h"

#include "ctu/sema/sema.h"

#include "std/str.h"
#include "std/vector.h"

#include "core/macros.h"

#include "memory/memory.h"
#include "base/panic.h"

static const char *attrib_name(const vector_t *path, arena_t *arena)
{
    return str_join("::", path, arena);
}

static ctu_attrib_t *get_attrib(tree_t *sema, const vector_t *path, arena_t *arena)
{
    CT_UNUSED(arena);

    CTASSERTF(vector_len(path) == 1, "expected 1 path element but got `%s`", attrib_name(path, arena));
    const char *name = vector_tail(path);

    return ctu_get_attrib(sema, name);
}

static ctu_attrib_t *attrib_create(const char *name, ctu_attrib_apply_t fn_apply, arena_t *arena)
{
    CTASSERT(name != NULL);
    CTASSERT(fn_apply != NULL);

    ctu_attrib_t *it = ARENA_MALLOC(sizeof(ctu_attrib_t), name, NULL, arena);
    it->name = name;
    it->fn_apply = fn_apply;
    return it;
}

static const char *get_first_string(tree_t *sema, tree_t *decl, const vector_t *args)
{
    if (vector_len(args) != 1)
    {
        msg_notify(sema->reports, &kEvent_IncorrectParamCount, decl->node, "expected 1 string argument");
        return NULL;
    }

    ctu_t *arg = vector_tail(args);
    if (arg->kind != eCtuExprString)
    {
        msg_notify(sema->reports, &kEvent_IncorrectParamType, arg->node, "expected string argument");
        return NULL;
    }

    return arg->text;
}

#define MALFORMED_ENTRY(REPORTS, NODE) msg_notify(REPORTS, &kEvent_MalformedAttribute, NODE, "malformed entry point type, must be either `gui` or `cli`")

static tree_linkage_t choose_linkage(tree_t *sema, const ctu_t *expr)
{
    if (expr->kind != eCtuExprName)
    {
        MALFORMED_ENTRY(sema->reports, expr->node);
        return eLinkEntryCli;
    }

    const vector_t *path = expr->path;
    if (vector_len(path) > 1)
    {
        MALFORMED_ENTRY(sema->reports, expr->node);
        return eLinkEntryCli;
    }

    const char *name = vector_tail(path);
    if (str_equal(name, "gui"))
    {
        return eLinkEntryGui;
    }

    if (str_equal(name, "cli"))
    {
        return eLinkEntryCli;
    }

    MALFORMED_ENTRY(sema->reports, expr->node);
    return eLinkEntryCli;
}

static tree_linkage_t get_linkage(tree_t *sema, tree_t *decl, const vector_t *args)
{
    switch (vector_len(args))
    {
    case 0: return eLinkEntryCli;
    case 1: return choose_linkage(sema, vector_tail(args));

    default:
        msg_notify(sema->reports, &kEvent_IncorrectParamCount, tree_get_node(decl), "entry attribute takes at most 1 argument, ignoring extra arguments");
        return eLinkEntryCli;
    }
}

///
/// attributes
///

static tree_attribs_t *dup_tree_attribs(const tree_attribs_t *attribs)
{
    arena_t *arena = get_global_arena();
    return arena_memdup(attribs, sizeof(tree_attribs_t), arena);
}

static void apply_entry(tree_t *sema, tree_t *decl, const vector_t *args)
{
    if (!tree_is(decl, eTreeDeclFunction))
    {
        msg_notify(sema->reports, &kEvent_InvalidAttributeApplication, decl->node, "entry attribute can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->link != eLinkModule && old->link != eLinkExport)
    {
        msg_notify(sema->reports, &kEvent_InvalidAttributeApplication, decl->node, "entry attribute can only be applied to public functions");
        return;
    }

    tree_attribs_t *copy = dup_tree_attribs(old);
    copy->link = get_linkage(sema, decl, args);
    tree_set_attrib(decl, copy);
}

static void apply_deprecated(tree_t *sema, tree_t *decl, const vector_t *args)
{
    if (!tree_is(decl, eTreeDeclFunction))
    {
        msg_notify(sema->reports, &kEvent_InvalidAttributeApplication, decl->node, "deprecated attribute can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->deprecated != NULL)
    {
        msg_notify(sema->reports, &kEvent_DuplicateAttribute, decl->node, "deprecated attribute already applied");
        return;
    }

    const char *msg = get_first_string(sema, decl, args);
    if (msg == NULL) { return; }

    tree_attribs_t *copy = dup_tree_attribs(old);
    copy->deprecated = msg;
    tree_set_attrib(decl, copy);
}

static void apply_section(tree_t *sema, tree_t *decl, const vector_t *args)
{
    if (!tree_is(decl, eTreeDeclFunction))
    {
        msg_notify(sema->reports, &kEvent_InvalidAttributeApplication, decl->node, "section attribute can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->section != NULL)
    {
        msg_notify(sema->reports, &kEvent_DuplicateAttribute, decl->node, "section attribute already applied");
        return;
    }

    const char *msg = get_first_string(sema, decl, args);
    if (msg == NULL) { return; }

    tree_attribs_t *copy = dup_tree_attribs(old);
    copy->section = msg;
    tree_set_attrib(decl, copy);
}

static void apply_extern(tree_t *sema, tree_t *decl, const vector_t *args)
{
    if (!tree_is(decl, eTreeDeclFunction))
    {
        msg_notify(sema->reports, &kEvent_InvalidAttributeApplication, decl->node, "`extern` can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->mangle != NULL)
    {
        msg_notify(sema->reports, &kEvent_DuplicateAttribute, decl->node, "`extern` already applied to this symbol");
        return;
    }

    tree_attribs_t *copy = dup_tree_attribs(old);
    copy->link = eLinkImport;
    if (vector_len(args) == 0)
    {
        copy->mangle = tree_get_name(decl);
    }
    else
    {
        const char *msg = get_first_string(sema, decl, args);
        if (msg == NULL) { return; }

        copy->mangle = msg;
    }
    tree_set_attrib(decl, copy);
}

static void apply_layout(tree_t *sema, tree_t *decl, const vector_t *args)
{
    CT_UNUSED(args);

    if (!tree_is(decl, eTreeTypeStruct))
    {
        msg_notify(sema->reports, &kEvent_InvalidAttributeApplication, decl->node, "layout attribute can only be applied to structs");
        return;
    }

    msg_notify(sema->reports, &kEvent_UnimplementedAttribute, tree_get_node(decl), "layout attribute not implemented");
}

void ctu_init_attribs(tree_t *sema, arena_t *arena)
{
    ctu_attrib_t *entry = attrib_create("entry", apply_entry, arena);
    tree_module_set(sema, eCtuTagAttribs, entry->name, entry);

    ctu_attrib_t *deprecated = attrib_create("deprecated", apply_deprecated, arena);
    tree_module_set(sema, eCtuTagAttribs, deprecated->name, deprecated);

    ctu_attrib_t *section = attrib_create("section", apply_section, arena);
    tree_module_set(sema, eCtuTagAttribs, section->name, section);

    ctu_attrib_t *attrib_extern = attrib_create("extern", apply_extern, arena);
    tree_module_set(sema, eCtuTagAttribs, attrib_extern->name, attrib_extern);

    ctu_attrib_t *layout = attrib_create("layout", apply_layout, arena);
    tree_module_set(sema, eCtuTagAttribs, layout->name, layout);
}

void ctu_apply_attribs(tree_t *sema, tree_t *decl, const vector_t *attribs)
{
    arena_t *arena = get_global_arena();
    size_t len = vector_len(attribs);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *attrib = vector_get(attribs, i);
        CTASSERTF(attrib->kind == eCtuAttrib, "expected attrib but got %d", attrib->kind);

        ctu_attrib_t *it = get_attrib(sema, attrib->attrib_path, arena);
        if (it == NULL)
        {
            msg_notify(sema->reports, &kEvent_AttribNotFound, attrib->node, "attrib '%s' not found", attrib_name(attrib->attrib_path, arena));
            continue;
        }

        it->fn_apply(sema, decl, attrib->attrib_args);
    }
}
