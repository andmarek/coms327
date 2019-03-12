#include <algorithm>
#include <thread>
#include "cerr.h"
#include "globs.h"

static void	dijkstra_d(void);
static void	dijkstra_dt(void);

constexpr static void	calc_cost_d(struct tile const *const, struct tile *const);
constexpr static void	calc_cost_dt(struct tile const *const, struct tile *const);

struct compare_d {
	constexpr bool
	operator() (struct tile const *const a, struct tile const *const b) const
	{
		return a->d > b->d;
	}
};

struct compare_dt {
	constexpr bool
	operator() (struct tile const *const a, struct tile const *const b) const
	{
		return a->dt > b->dt;
	}
};

void
dijkstra(void)
{
	std::thread t1(dijkstra_d);
	std::thread t2(dijkstra_dt);

	t1.join();
	t2.join();
}

static void
dijkstra_d(void)
{
	struct tile *t;
	std::size_t i, j;

	std::vector<struct tile *> heap;

	for (i = 1; i < HEIGHT - 1; ++i) {
		for (j = 1; j < WIDTH - 1; ++j) {
			tiles[i][j].d = std::numeric_limits<int32_t>::max();

			if (tiles[i][j].h == 0) {
				tiles[i][j].vd = true;
				heap.push_back(&tiles[i][j]);
			}
		}
	}

	tiles[player.y][player.x].d = 0;

	while(!heap.empty()) {
		std::make_heap(heap.begin(), heap.end(), compare_d());

		t = heap.front();
		std::pop_heap(heap.begin(), heap.end(), compare_d());
		heap.pop_back();

		calc_cost_d(t, &tiles[t->y - 1][t->x + 0]);
		calc_cost_d(t, &tiles[t->y + 1][t->x + 0]);

		calc_cost_d(t, &tiles[t->y + 0][t->x - 1]);
		calc_cost_d(t, &tiles[t->y + 0][t->x + 1]);

		calc_cost_d(t, &tiles[t->y + 1][t->x + 1]);
		calc_cost_d(t, &tiles[t->y - 1][t->x - 1]);

		calc_cost_d(t, &tiles[t->y - 1][t->x + 1]);
		calc_cost_d(t, &tiles[t->y + 1][t->x - 1]);

		t->vd = false;
	}
}

static void
dijkstra_dt(void)
{
	struct tile *t;
	std::size_t i, j;

	std::vector<struct tile *> heap;
	heap.reserve((HEIGHT - 1) * (WIDTH - 1));

	for (i = 1; i < HEIGHT - 1; ++i) {
		for (j = 1; j < WIDTH - 1; ++j) {
			tiles[i][j].dt = std::numeric_limits<int32_t>::max();
			tiles[i][j].vdt = true;
			heap.push_back(&tiles[i][j]);
		}
	}

	tiles[player.y][player.x].dt = 0;

	while(!heap.empty()) {
		std::make_heap(heap.begin(), heap.end(), compare_dt());

		t = heap.front();
		std::pop_heap(heap.begin(), heap.end(), compare_dt());
		heap.pop_back();

		calc_cost_dt(t, &tiles[t->y - 1][t->x + 0]);
		calc_cost_dt(t, &tiles[t->y + 1][t->x + 0]);

		calc_cost_dt(t, &tiles[t->y + 0][t->x - 1]);
		calc_cost_dt(t, &tiles[t->y + 0][t->x + 1]);

		calc_cost_dt(t, &tiles[t->y + 1][t->x + 1]);
		calc_cost_dt(t, &tiles[t->y - 1][t->x - 1]);

		calc_cost_dt(t, &tiles[t->y - 1][t->x + 1]);
		calc_cost_dt(t, &tiles[t->y + 1][t->x - 1]);

		t->vdt = false;
	}
}

constexpr static void
calc_cost_d(struct tile const *const a, struct tile *const b)
{
	if (b->vd && b->d > a->d) {
		b->d = a->d + 1;
	}
}

constexpr static void
calc_cost_dt(struct tile const *const a, struct tile *const b)
{
	if (b->vdt && b->dt > a->dt + a->h/85) {
		b->dt = a->dt + 1 + a->h/85;
	}
}