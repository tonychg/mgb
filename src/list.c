#include "gb/list.h"
#include <stdlib.h>

struct list *list_create(void)
{
	struct list *list;

	if ((list = (struct list *)malloc(sizeof(struct list))) == NULL)
		return NULL;
	list->head = NULL;
	list->free = NULL;
	list->len = 0;
	return list;
}

void list_link_node_head(struct list *list, struct list_node *node)
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

struct list *list_add_node_head(struct list *list, void *data)
{
	struct list_node *node;

	if ((node = (struct list_node *)malloc(sizeof(struct list_node))) ==
	    NULL)
		return NULL;
	node->data = data;
	list_link_node_head(list, node);
	return list;
}

void list_link_node_tail(struct list *list, struct list_node *node)
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

struct list *list_add_node_tail(struct list *list, void *data)
{
	struct list_node *node;

	if ((node = (struct list_node *)malloc(sizeof(struct list_node))) ==
	    NULL)
		return NULL;
	node->data = data;
	list_link_node_tail(list, node);
	return list;
}

void list_unlink(struct list *list, struct list_node *node)
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

struct list_node *list_pop_node_head(struct list *list)
{
	struct list_node *node;

	node = list->head;
	list_unlink(list, node);
	return node;
}

struct list_node *list_pop_node_tail(struct list *list)
{
	struct list_node *node;

	node = list->tail;
	list_unlink(list, node);
	return node;
}

void list_empty(struct list *list)
{
	unsigned long len;
	struct list_node *next, *current;

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

void list_release(struct list *list)
{
	if (list == NULL)
		return;
	list_empty(list);
	free(list);
}

struct list_iterator *list_iter(struct list *list, int direction)
{
	struct list_iterator *it;

	if ((it = (struct list_iterator *)malloc(
		     sizeof(struct list_iterator))) == NULL)
		return NULL;
	it->direction = direction;
	if (direction == IT_FORWARD)
		it->next = list->head;
	else
		it->next = list->tail;
	return it;
}

struct list_node *list_next(struct list_iterator *it)
{
	struct list_node *current = it->next;

	if (current != NULL) {
		if (it->direction == IT_FORWARD)
			it->next = current->next;
		else
			it->next = current->prev;
	}
	return current;
}

void list_release_iter(struct list_iterator *it)
{
	free(it);
}

#ifdef TEST_MAIN
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_list()
{
	struct list_node *node, *head, *tail;
	struct list_iterator *it;
	struct list *list = list_create();
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
