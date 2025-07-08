#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
#include <stdbool.h>

typedef struct ListNode {
	void *data;
	struct ListNode *next;
	struct ListNode *prev;
} ListNode;

typedef struct List {
	ListNode *head;
	ListNode *tail;
	void (*free)(void *ptr);
	unsigned long len;
} List;

typedef struct ListIterator {
	ListNode *next;
	int direction;
} ListIterator;

List *list_create(void);
void list_link_node_head(List *list, ListNode *node);
List *list_add_node_head(List *list, void *data);
void list_link_node_tail(List *list, ListNode *node);
List *list_add_node_tail(List *list, void *data);
ListNode *list_pop_node_head(List *list);
ListNode *list_pop_node_tail(List *list);
void list_empty(List *list);
void list_release(List *list);
ListIterator *list_iter(List *list, int direction);
ListNode *list_next(ListIterator *it);
void list_release_iter(ListIterator *it);

#define IT_FORWARD 0
#define IT_BACKWARD 1

#endif
