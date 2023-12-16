#include "std/list.h"

#include "base/panic.h"
#include "memory/memory.h"

typedef struct list_t
{
    list_node_t *head;
    list_node_t *tail;
} list_t;

typedef struct list_node_t
{
    void *value;
    struct list_node_t *next;
    struct list_node_t *prev;
} list_node_t;

static list_node_t *list_node_new(void *value, list_node_t *prev, list_node_t *next)
{
    list_node_t *node = MEM_ALLOC(sizeof(list_node_t), "list_node", NULL);
    node->value = value;
    node->prev = prev;
    node->next = next;
    return node;
}

static list_t *list_new_inner(list_node_t *head, list_node_t *tail)
{
    list_t *list = MEM_ALLOC(sizeof(list_t), "list", NULL);
    list->head = head;
    list->tail = tail;
    return list;
}

USE_DECL
list_t *list_new(void)
{
    return list_new_inner(NULL, NULL);
}

USE_DECL
list_t *list_init(void *value)
{
    list_node_t *node = list_node_new(value, NULL, NULL);
    return list_new_inner(node, node);
}

USE_DECL
list_node_t *list_head(list_t *list)
{
    CTASSERT(list != NULL);

    return list->head;
}

USE_DECL
list_node_t *list_tail(list_t *list)
{
    CTASSERT(list != NULL);

    return list->tail;
}

USE_DECL
list_node_t *list_append(list_t *list, void *value)
{
    CTASSERT(list != NULL);

    list_node_t *tail = list_tail(list);

    list_node_t *node = list_node_new(value, tail, NULL);

    if (tail != NULL)
    {
        tail->next = node;
    }

    list->tail = node;

    if (list->head == NULL)
    {
        list->head = node;
    }

    return node;
}

USE_DECL
list_node_t *list_prepend(list_t *list, void *value)
{
    CTASSERT(list != NULL);

    list_node_t *head = list_head(list);

    list_node_t *node = list_node_new(value, NULL, head);

    if (head != NULL)
    {
        head->prev = node;
    }

    list->head = node;

    if (list->tail == NULL)
    {
        list->tail = node;
    }

    return node;
}

USE_DECL
void list_remove(list_t *list, list_node_t *node)
{
    CTASSERT(list != NULL);
    CTASSERT(node != NULL);

    list_node_t *prev = node->prev;
    list_node_t *next = node->next;

    if (prev != NULL)
    {
        prev->next = next;
    }

    if (next != NULL)
    {
        next->prev = prev;
    }

    if (list->head == node)
    {
        list->head = next;
    }

    if (list->tail == node)
    {
        list->tail = prev;
    }

    ctu_free(node);
}

USE_DECL
list_node_t *list_next(list_node_t *node)
{
    CTASSERT(node != NULL);

    return node->next;
}

USE_DECL
list_node_t *list_prev(list_node_t *node)
{
    CTASSERT(node != NULL);

    return node->prev;
}

USE_DECL
void *list_value(list_node_t *node)
{
    CTASSERT(node != NULL);

    return node->value;
}
