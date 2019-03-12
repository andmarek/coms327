#ifndef HEAP_H
#define HEAP_H

#include <cstdint>

struct heap_node {
	struct heap_node	*next;
	struct heap_node	*prev;
	struct heap_node	*parent;
	struct heap_node	*child;
	void			*datum;
	uint32_t		degree;
	uint32_t		mark;
};

struct heap {
	struct heap_node	*min;
	uint32_t		size;
	int32_t			(*compare)(void const *const, void const *const);
	void			(*datum_delete)(void const *const);
};

void			heap_init(struct heap *const, int32_t (*const compare)(void const *const, void const *const), void (*const datum_delete)(void const *const));
void			heap_delete(struct heap *const);
struct heap_node	*heap_insert(struct heap *const, void *const);
void			*heap_remove_min(struct heap *const);
int			heap_decrease_key_no_replace(struct heap *const, struct heap_node *const);

#endif /* HEAP_H */
