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

/// @brief create a new list
///
/// @return the new list
NODISCARD
list_t *list_new(void);

/// @brief create a new list with an initial value
///
/// @param value the initial value
///
/// @return the new list
NODISCARD
list_t *list_init(void *value);

/// @brief get the first node in a list
///
/// @param list the list to get the head of
///
/// @return the head of @p list
list_node_t *list_head(IN_NOTNULL list_t *list);

/// @brief get the last node in a list
///
/// @param list the list to get the tail of
///
/// @return the tail of @p list
list_node_t *list_tail(IN_NOTNULL list_t *list);

/// @brief add a value to the end of a list
///
/// @param list the list to append to
/// @param value the value to append
///
/// @return the new node
list_node_t *list_append(IN_NOTNULL list_t *list, void *value);

/// @brief add a value to the start of a list
///
/// @param list the list to prepend to
/// @param value the value to prepend
///
/// @return the new node
list_node_t *list_prepend(IN_NOTNULL list_t *list, void *value);

/// @brief remove a node from a list
///
/// @param list the list to remove from
/// @param node the node to remove
void list_remove(IN_NOTNULL list_t *list, IN_NOTNULL list_node_t *node);

/// @brief get the next node in a list
///
/// @param node the node to get the next node of
///
/// @return the next node of @p node
list_node_t *list_next(IN_NOTNULL list_node_t *node);

/// @brief get the previous node in a list
///
/// @param node the node to get the previous node of
///
/// @return the previous node of @p node
list_node_t *list_prev(IN_NOTNULL list_node_t *node);

/// @brief get the value of a node
///
/// @param node the node to get the value of
///
/// @return the value of @p node
void *list_value(IN_NOTNULL list_node_t *node);

/// @}

END_API
