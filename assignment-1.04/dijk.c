#include "cerr.h"
#include "heap.h"
#include "opal.h"

static int32_t	compare_nontunnel(void const *const, void const *const);
static int32_t	compare_tunnel(void const *const, void const *const);

static void	calc_cost_nontunnel(struct heap *const, struct heap_node *const, struct tile const *const, struct tile *const);
static void	calc_cost_tunnel(struct heap *const, struct heap_node *const, struct tile const *const, struct tile *const);

int
dijkstra(int const w, int const h, int const py, int const px)
{
	struct heap_node *nodes[HEIGHT][WIDTH];
	struct heap heap;
	struct tile *t;
	int i, j;

	tiles[py][px].d = 0;
	tiles[py][px].dt = 0;

	heap_init(&heap, compare_nontunnel, NULL);

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			if (i != py || j != px) {
				tiles[i][j].d = INT32_MAX;
				tiles[i][j].dt = INT32_MAX;
			}

			if (tiles[i][j].h != 0) {
				nodes[i][j] = NULL;
			} else {
				nodes[i][j] = heap_insert(&heap, &tiles[i][j]);
				if (nodes[i][j] == NULL) {
					cerr(1, "dijk nontunnel heap_insert");
				}
			}
		}
	}

#define CALC_NONTUNNEL(y,x)	calc_cost_nontunnel(&heap, nodes[(y)][(x)], t, &tiles[(y)][(x)])

	while((t = heap_remove_min(&heap))) {
		CALC_NONTUNNEL(t->y-1, t->x+0);
		CALC_NONTUNNEL(t->y+1, t->x+0);

		CALC_NONTUNNEL(t->y+0, t->x-1);
		CALC_NONTUNNEL(t->y+0, t->x+1);

		CALC_NONTUNNEL(t->y+1, t->x+1);
		CALC_NONTUNNEL(t->y-1, t->x-1);

		CALC_NONTUNNEL(t->y-1, t->x+1);
		CALC_NONTUNNEL(t->y+1, t->x-1);
	}

	heap_delete(&heap);

	heap_init(&heap, compare_tunnel, NULL);

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			if (tiles[i][j].h == UINT8_MAX) {
				nodes[i][j] = NULL;
			} else {
				nodes[i][j] = heap_insert(&heap, &tiles[i][j]);
				if (nodes[i][j] == NULL) {
					cerr(1, "dijk tunnel heap_insert");
				}
			}
		}
	}

#define CALC_TUNNEL(y,x)	calc_cost_tunnel(&heap, nodes[y][x], t, &tiles[y][x])

	while((t = heap_remove_min(&heap))) {
		CALC_TUNNEL(t->y-1, t->x+0);
		CALC_TUNNEL(t->y+1, t->x+0);

		CALC_TUNNEL(t->y+0, t->x-1);
		CALC_TUNNEL(t->y+0, t->x+1);

		CALC_TUNNEL(t->y+1, t->x+1);
		CALC_TUNNEL(t->y-1, t->x-1);

		CALC_TUNNEL(t->y-1, t->x+1);
		CALC_TUNNEL(t->y+1, t->x-1);
	}

	heap_delete(&heap);

	return 0;
}

static int32_t
compare_nontunnel(void const *const key, void const *const with)
{
	return ((struct tile const *const) key)->d - ((struct tile const *const) with)->d;
}

static int32_t
compare_tunnel(void const *const key, void const *const with)
{
	return ((struct tile const *const) key)->dt - ((struct tile const *const) with)->dt;
}

static void
calc_cost_nontunnel(struct heap *const h, struct heap_node *const n,
	struct tile const *const t1, struct tile *const t2)
{
	if (n != NULL && t2->d > t1->d) {
		t2->d = t1->d + 1;
		(void)heap_decrease_key_no_replace(h, n);
	}
}

static void
calc_cost_tunnel(struct heap *const h, struct heap_node *const n,
	struct tile const *const t1, struct tile *const t2)
{
	if (n != NULL && t2->dt > t1->dt + t1->h/85) {
		t2->dt = t1->dt + 1 + t1->h/85;
		(void)heap_decrease_key_no_replace(h, n);
	}
}

