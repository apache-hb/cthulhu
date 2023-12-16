#pragma once

#include "core/analyze.h"
#include "core/compiler.h"

BEGIN_API

/// @defgroup List Linked list
/// @ingroup Standard
/// @brief Doubly linked list
/// @{

typedef struct list_t list_t;
typedef struct list_node_t list_node_t;

NODISCARD
list_t *list_new(void);

NODISCARD
list_t *list_init(void *value);

list_node_t *list_head(IN_NOTNULL list_t *list);
list_node_t *list_tail(IN_NOTNULL list_t *list);

list_node_t *list_append(IN_NOTNULL list_t *list, void *value);
list_node_t *list_prepend(IN_NOTNULL list_t *list, void *value);

void list_remove(IN_NOTNULL list_t *list, IN_NOTNULL list_node_t *node);

list_node_t *list_next(IN_NOTNULL list_node_t *node);
list_node_t *list_prev(IN_NOTNULL list_node_t *node);

void *list_value(IN_NOTNULL list_node_t *node);

/// @}

END_API
