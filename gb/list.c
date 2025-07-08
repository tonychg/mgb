#include "gb/list.h"

List *list_create(void)
{
	List *list;

	if ((list = (List *)malloc(sizeof(List))) == NULL)
		return NULL;
	list->head = NULL;
	list->free = NULL;
	list->len = 0;
	return list;
}

void list_link_node_head(List *list, ListNode *node)
{
	if (list->len == 0) {
		list->head = list->tail = node;
		node->prev = node->next = NULL;
	} else {
		node->prev = NULL;
		node->next = list->head;
		list->head->prev = node;
		list->head = node;
	}
	list->len++;
}

List *list_add_node_head(List *list, void *data)
{
	ListNode *node;

	if ((node = (ListNode *)malloc(sizeof(ListNode))) == NULL)
		return NULL;
	node->data = data;
	list_link_node_head(list, node);
	return list;
}

void list_link_node_tail(List *list, ListNode *node)
{
	if (list->len == 0) {
		list->head = list->tail = node;
		node->prev = node->next = NULL;
	} else {
		node->prev = list->tail;
		node->next = NULL;
		list->tail->next = node;
		list->tail = node;
	}
	list->len++;
}

List *list_add_node_tail(List *list, void *data)
{
	ListNode *node;

	if ((node = (ListNode *)malloc(sizeof(ListNode))) == NULL)
		return NULL;
	node->data = data;
	list_link_node_tail(list, node);
	return list;
}

void list_unlink(List *list, ListNode *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		list->head = node->next;
	if (node->next)
		node->next->prev = node->prev;
	else
		list->tail = node->prev;
	node->next = NULL;
	node->prev = NULL;
	list->len--;
}

ListNode *list_pop_node_head(List *list)
{
	ListNode *node;

	node = list->head;
	list_unlink(list, node);
	return node;
}

ListNode *list_pop_node_tail(List *list)
{
	ListNode *node;

	node = list->tail;
	list_unlink(list, node);
	return node;
}

void list_empty(List *list)
{
	unsigned long len;
	ListNode *next, *current;

	current = list->head;
	len = list->len;
	while (len--) {
		next = current->next;
		if (list->free != NULL)
			list->free(current->data);
		free(current);
		current = next;
	}
	list->head = list->tail = NULL;
	list->len = 0;
}

void list_release(List *list)
{
	if (list == NULL)
		return;
	list_empty(list);
	free(list);
}

ListIterator *list_iter(List *list, int direction)
{
	ListIterator *it;

	if ((it = (ListIterator *)malloc(sizeof(ListIterator))) == NULL)
		return NULL;
	it->direction = direction;
	if (direction == IT_FORWARD)
		it->next = list->head;
	else
		it->next = list->tail;
	return it;
}

ListNode *list_next(ListIterator *it)
{
	ListNode *current = it->next;

	if (current != NULL) {
		if (it->direction == IT_FORWARD)
			it->next = current->next;
		else
			it->next = current->prev;
	}
	return current;
}

void list_release_iter(ListIterator *it)
{
	free(it);
}

#ifdef TEST_MAIN
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_list()
{
	ListNode *node, *head, *tail;
	ListIterator *it;
	List *list = list_create();
	list_add_node_head(list, "1");
	list_add_node_head(list, "2");
	list_add_node_head(list, "3");
	it = list_iter(list, IT_FORWARD);
	while ((node = list_next(it)) != NULL)
		printf("forward: %s\n", (char *)node->data);
	list_release_iter(it);
	it = list_iter(list, IT_BACKWARD);
	while ((node = list_next(it)) != NULL)
		printf("backward: %s\n", (char *)node->data);
	list_release_iter(it);
	head = list_pop_node_head(list);
	assert(strcmp((char *)head->data, "3") == 0);
	tail = list_pop_node_tail(list);
	assert(strcmp((char *)tail->data, "1") == 0);
	free(head);
	free(tail);
	list_release(list);
}
#endif
