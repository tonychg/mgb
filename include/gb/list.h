#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>

struct list_node {
	void *data;
	struct list_node *next;
	struct list_node *prev;
};

struct list {
	struct list_node *head;
	struct list_node *tail;
	void (*free)(void *ptr);
	unsigned long len;
};

struct list_iterator {
	struct list_node *next;
	int direction;
};

struct list *list_create(void);
void list_link_node_head(struct list *list, struct list_node *node);
struct list *list_add_node_head(struct list *list, void *data);
void list_link_node_tail(struct list *list, struct list_node *node);
struct list *list_add_node_tail(struct list *list, void *data);
struct list_node *list_pop_node_head(struct list *list);
struct list_node *list_pop_node_tail(struct list *list);
void list_empty(struct list *list);
void list_release(struct list *list);
struct list_iterator *list_iter(struct list *list, int direction);
struct list_node *list_next(struct list_iterator *it);
void list_release_iter(struct list_iterator *it);

#define IT_FORWARD 0
#define IT_BACKWARD 1

#endif
