#include "unit/ct-test.h"

#include "std/list.h"


int main()
{
    test_install_panic_handler();

    test_suite_t suite = test_suite_new("list");

    {
        test_group_t group = test_group(&suite, "construction");
        GROUP_EXPECT_PASS(group, "not null", list_new() != NULL);
    }

    {
        test_group_t group = test_group(&suite, "init1");
        list_t *l1 = list_init((void*)1);
        list_node_t *head = list_head(l1);
        list_node_t *tail = list_tail(l1);

        GROUP_EXPECT_PASS(group, "not null", l1 != NULL);
        GROUP_EXPECT_PASS(group, "head", list_value(head) == (void*)1);
        GROUP_EXPECT_PASS(group, "tail", list_value(tail) == (void*)1);
    }

    {
        test_group_t group = test_group(&suite, "init2");
        list_t *l2 = list_new();

        list_node_t *head = list_head(l2);
        list_node_t *tail = list_tail(l2);

        GROUP_EXPECT_PASS(group, "not null", l2 != NULL);
        GROUP_EXPECT_PASS(group, "head", head == NULL);
        GROUP_EXPECT_PASS(group, "tail", tail == NULL);
    }

    {
        test_group_t group = test_group(&suite, "insertion");

        list_t *list = list_new();
        list_node_t *node1 = list_append(list, (void*)1);
        list_node_t *node2 = list_append(list, (void*)2);
        list_node_t *node3 = list_append(list, (void*)3);

        list_node_t *head = list_head(list);
        list_node_t *tail = list_tail(list);

        GROUP_EXPECT_PASS(group, "not null", list != NULL);
        GROUP_EXPECT_PASS(group, "head", list_value(head) == (void*)1);
        GROUP_EXPECT_PASS(group, "tail", list_value(tail) == (void*)3);
        GROUP_EXPECT_PASS(group, "node1", list_value(node1) == (void*)1);
        GROUP_EXPECT_PASS(group, "node2", list_value(node2) == (void*)2);
        GROUP_EXPECT_PASS(group, "node3", list_value(node3) == (void*)3);

        list_node_t *node4 = list_prepend(list, (void*)4);
        list_node_t *node5 = list_prepend(list, (void*)5);
        list_node_t *node6 = list_prepend(list, (void*)6);

        head = list_head(list);
        tail = list_tail(list);

        GROUP_EXPECT_PASS(group, "not null", list != NULL);
        GROUP_EXPECT_PASS(group, "head", list_value(head) == (void*)6);
        GROUP_EXPECT_PASS(group, "tail", list_value(tail) == (void*)3);
        GROUP_EXPECT_PASS(group, "node4", list_value(node4) == (void*)4);
        GROUP_EXPECT_PASS(group, "node5", list_value(node5) == (void*)5);
        GROUP_EXPECT_PASS(group, "node6", list_value(node6) == (void*)6);
    }

    {
        test_group_t group = test_group(&suite, "iteration");

        list_t *list = list_new();
        list_append(list, (void*)1);
        list_append(list, (void*)2);
        list_append(list, (void*)3);
        list_prepend(list, (void*)4);
        list_prepend(list, (void*)5);
        list_prepend(list, (void*)6);

        list_node_t *head = list_head(list);
        GROUP_EXPECT_PASS(group, "not null", list != NULL);
        GROUP_EXPECT_PASS(group, "head", list_value(head) == (void*)6);

        list_node_t *node = list_next(head);
        GROUP_EXPECT_PASS(group, "node", list_value(node) == (void*)5);

        node = list_next(node);
        GROUP_EXPECT_PASS(group, "node", list_value(node) == (void*)4);

        node = list_next(node);
        GROUP_EXPECT_PASS(group, "node", list_value(node) == (void*)1);

        node = list_next(node);
        GROUP_EXPECT_PASS(group, "node", list_value(node) == (void*)2);
    }

    {
        test_group_t group = test_group(&suite, "deletion");

        list_t *list = list_new();
        list_append(list, (void*)1);
        list_append(list, (void*)2);
        list_append(list, (void*)3);

        list_node_t *head = list_head(list);
        list_node_t *tail = list_tail(list);

        list_node_t *node = list_next(head);
        list_remove(list, head);
        GROUP_EXPECT_PASS(group, "not null", list != NULL);
        GROUP_EXPECT_PASS(group, "head", list_value(list_head(list)) == (void*)2);
        GROUP_EXPECT_PASS(group, "tail", list_value(tail) == (void*)3);

        list_remove(list, node);
        GROUP_EXPECT_PASS(group, "head", list_value(list_head(list)) == (void*)3);
    }

    return test_suite_finish(&suite);
}
