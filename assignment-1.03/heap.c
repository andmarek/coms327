#include <stdlib.h>
#include <string.h>

#include "heap.h"

#define SPLICE_LISTS(n1, n2) do {		\
	if ((n1) != NULL && (n2) != NULL) {	\
		(n1)->next->prev = (n2)->prev;	\
		(n2)->prev->next = (n1)->next;	\
		(n1)->next = (n2);		\
		(n2)->prev = (n1);		\
	}					\
} while(0)

#define INSERT_NODE(n, l) do {	\
	(n)->next = (l);	\
	(n)->prev = (l)->prev;	\
	(n)->prev->next = (n);	\
	(l)->prev = (n);	\
} while(0)

#define REMOVE_NODE(n) do {		\
	(n)->next->prev = (n)->prev;	\
	(n)->prev->next = (n)->next;	\
} while(0)

void
heap_init(struct heap *const h,
	int32_t (*const compare)(void const *const key, void const *const with),
	void (*const datum_delete)(void const *const))
{
	h->min = NULL;
	h->size = 0;
	h->compare = compare;
	h->datum_delete = datum_delete;
}

static void
heap_node_delete(struct heap const *const h, struct heap_node *hn)
{
	struct heap_node *next;

	hn->prev->next = NULL;

	while (hn != NULL) {
		if (hn->child != NULL) {
			heap_node_delete(h, hn->child);
		}

		next = hn->next;

		if (h->datum_delete != NULL) {
			h->datum_delete(hn->datum);
		}

		free(hn);
		hn = next;
	}
}

void
heap_delete(struct heap *const h)
{
	if (h->min != NULL) {
		heap_node_delete(h, h->min);
	}

	h->min = NULL;
	h->size = 0;
	h->compare = NULL;
	h->datum_delete = NULL;
}

struct heap_node *
heap_insert(struct heap *const h, void *const v)
{
	struct heap_node *const n = calloc(1, sizeof(struct heap_node));

	if (n == NULL) {
		return NULL;
	}

	n->datum = v;

	if (h->min != NULL) {
		INSERT_NODE(n, h->min);
	} else {
		n->next = n->prev = n;
	}

	if (h->min == NULL || (h->compare(v, h->min->datum) < 0)) {
		h->min = n;
	}

	h->size++;

	return n;
}

static void
heap_link(struct heap_node *const node, struct heap_node *const root)
{
	if (root->child != NULL) {
		INSERT_NODE(node, root->child);
	} else {
		root->child = node;
		node->next = node->prev = node;
	}

	node->parent = root;
	root->degree++;
	node->mark = 0;
}

static void
heap_consolidate(struct heap *const h)
{
#define BUFSIZE	64 /* sufficient for 64-bit address space, ceil(lg(h->size)) */

	uint32_t i;
	struct heap_node *x, *y, *n, *tmp;
	struct heap_node *a[BUFSIZE];

	memset(a, 0, sizeof(struct heap_node *[BUFSIZE]));

	h->min->prev->next = NULL;

	for (x = n = h->min; n != NULL; x = n) {
		n = n->next;

		while (a[x->degree] != NULL) {
			y = a[x->degree];

			if (h->compare(x->datum, y->datum) > 0) {
				tmp = x;
				x = y;
				y = tmp;
			}

			a[x->degree] = NULL;
			heap_link(y, x);
		}

		a[x->degree] = x;
	}

	for (h->min = NULL, i = 0; i < BUFSIZE; ++i) {
		if (a[i] == NULL) {
			continue;
		}

		if (h->min != NULL) {
			INSERT_NODE(a[i], h->min);

			if (h->compare(a[i]->datum, h->min->datum) < 0) {
				h->min = a[i];
			}
		} else {
			h->min = a[i];
			a[i]->next = a[i]->prev = a[i];
		}
	}
}

void *
heap_remove_min(struct heap *const h)
{
	void *v;
	struct heap_node *n;

	if (h->min == NULL) {
		return NULL;
	}

	v = h->min->datum;

	if (h->size == 1) {
		free(h->min);
		h->min = NULL;
	} else {
		if ((n = h->min->child)) {
			for (; n->parent != NULL; n = n->next) {
				n->parent = NULL;
			}
		}

		SPLICE_LISTS(h->min, h->min->child);

		n = h->min;
		REMOVE_NODE(n);
		h->min = n->next;
		free(n);

		heap_consolidate(h);
	}

	h->size--;

	return v;
}

static void
heap_cut(struct heap *const h, struct heap_node *const n,
	struct heap_node *const p)
{
	if (--p->degree == 0) {
		p->child = NULL;
	}

	if (p->child == n) {
		p->child = p->child->next;
	}

	REMOVE_NODE(n);
	n->parent = NULL;
	n->mark = 0;
	INSERT_NODE(n, h->min);
}

static void
heap_cascading_cut(struct heap *const h, struct heap_node *const n)
{
	struct heap_node *const p = n->parent;

	if (p == NULL) {
		return;
	}

	if (n->mark == 0) {
		n->mark = 1;
	} else {
		heap_cut(h, n, p);
		heap_cascading_cut(h, n);
	}
}

int
heap_decrease_key_no_replace(struct heap *const h, struct heap_node *const n)
{
	struct heap_node *const p = n->parent;

	if (p != NULL && (h->compare(n->datum, p->datum) < 0)) {
		heap_cut(h, n, p);
		heap_cascading_cut(h, p);
	}

	if (h->compare(n->datum, h->min->datum) < 0) {
		h->min = n;
	}

	return 0;
}
