#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>

typedef struct list_node {
	void *data;
	struct list_node *next;
	struct list_node *prev;
} list_node;

typedef struct list {
	list_node *head;
	list_node *tail;
	void (*free)(void *ptr);
	unsigned long len;
} list;

typedef struct list_iterator {
	list_node *next;
	int direction;
} list_iterator;

list *list_create(void);
void list_link_node_head(list *list, list_node *node);
list *list_add_node_head(list *list, void *data);
void list_link_node_tail(list *list, list_node *node);
list *list_add_node_tail(list *list, void *data);
list_node *list_pop_node_head(list *list);
list_node *list_pop_node_tail(list *list);
void list_empty(list *list);
void list_release(list *list);
list_iterator *list_iter(list *list, int direction);
list_node *list_next(list_iterator *it);
void list_release_iter(list_iterator *it);

#define IT_FORWARD  0
#define IT_BACKWARD 1

#endif
