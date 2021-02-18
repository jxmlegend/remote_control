#ifndef __LIST_H__
#define __LIST_H__

typedef struct list_node{
	struct list_node *prev;
	struct list_node *next;
	void *value;
}list_node;

typedef struct list_iter{
	list_node *next;
	int direction;
}list_iter;

typedef struct list{
	list_node *head;
	list_node *tail;
	void *(*dup)(void *ptr);
	void (*free)(void *ptr);
	int (*match)(void *ptr, void *key);
	unsigned long len;	
}list;

/* Functions implemented as macros */
#define list_length(l) ((l)->len)
#define list_first(l) ((l)->head)
#define list_last(l) ((l)->tail)
#define list_prev_node(l) ((l)->prev)
#define list_next_node(l) ((l)->next)
#define list_node_value(l) ((l)->value)

#define list_set_dup_method(l) ((l)->dup = (m))
#define list_set_free_method(l) ((l)->free = (m))
#define list_set_match_method(l) ((l)->match = (m))

#define list_get_dup_method(l) ((l)->dup)
#define list_get_free_method(l) ((l)->free)
#define list_get_match_method(l) ((l)->match)

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1


#endif //__LIST_H__
