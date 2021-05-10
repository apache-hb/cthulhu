#include "ast.h"

#include <stdlib.h>

static node_t* new_node(node_type_t type)
{
    node_t *node = malloc(sizeof(node_t));
    node->type = type;
    node->mut = false;
    node->exported = false;
    node->loc = NULL;
    return node;
}

node_t *new_digit(char *digit, char *suffix)
{
    node_t *node = new_node(NODE_DIGIT);
    node->digit.digit = digit;
    node->digit.suffix = suffix;
    return node;
}

node_t *new_string(char *text)
{
    node_t *node = new_node(NODE_STRING);
    node->text = text;
    return node;
}

node_t *new_null()
{
    return new_node(NODE_NULL);
}

node_t *loc(node_t *node, struct YYLTYPE *loc)
{
    node->loc = loc;
    return node;
}

