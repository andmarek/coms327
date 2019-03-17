#include <algorithm>
#include <functional>
#include <thread>
#include "cerr.h"
#include "globs.h"

static void	dijkstra_d();
static void	dijkstra_dt();

static void constexpr	calc_cost_d(tile const &, tile &);
static void constexpr	calc_cost_dt(tile const &, tile &);

struct compare_d {
	bool constexpr
	operator() (tile const &a, tile const &b) const
	{
		return a.d > b.d;
	}
};

struct compare_dt {
	bool constexpr
	operator() (tile const &a, tile const &b) const
	{
		return a.dt > b.dt;
	}
};

void
dijkstra()
{
	std::thread t1(dijkstra_d);
	std::thread t2(dijkstra_dt);

	t1.join();
	t2.join();
}

static void
dijkstra_d()
{
	std::vector<std::reference_wrapper<tile>> heap;

	for (std::size_t i = 1; i < HEIGHT - 1; ++i) {
		for (std::size_t j = 1; j < WIDTH - 1; ++j) {
			tiles[i][j].d = std::numeric_limits<int32_t>::max();

			if (tiles[i][j].h == 0) {
				tiles[i][j].vd = true;
				heap.push_back(tiles[i][j]);
			}
		}
	}

	tiles[player.y][player.x].d = 0;

	while(!heap.empty()) {
		std::make_heap(heap.begin(), heap.end(), compare_d());

		tile &t = heap.front();
		std::pop_heap(heap.begin(), heap.end(), compare_d());
		heap.pop_back();

		calc_cost_d(t, tiles[t.y - 1][t.x + 0]);
		calc_cost_d(t, tiles[t.y + 1][t.x + 0]);

		calc_cost_d(t, tiles[t.y + 0][t.x - 1]);
		calc_cost_d(t, tiles[t.y + 0][t.x + 1]);

		calc_cost_d(t, tiles[t.y + 1][t.x + 1]);
		calc_cost_d(t, tiles[t.y - 1][t.x - 1]);

		calc_cost_d(t, tiles[t.y - 1][t.x + 1]);
		calc_cost_d(t, tiles[t.y + 1][t.x - 1]);

		t.vd = false;
	}
}

static void
dijkstra_dt()
{
	std::vector<std::reference_wrapper<tile>> heap;
	heap.reserve((HEIGHT - 1) * (WIDTH - 1));

	for (std::size_t i = 1; i < HEIGHT - 1; ++i) {
		for (std::size_t j = 1; j < WIDTH - 1; ++j) {
			tiles[i][j].dt = std::numeric_limits<int32_t>::max();
			tiles[i][j].vdt = true;
			heap.push_back(tiles[i][j]);
		}
	}

	tiles[player.y][player.x].dt = 0;

	while(!heap.empty()) {
		std::make_heap(heap.begin(), heap.end(), compare_dt());

		tile &t = heap.front();
		std::pop_heap(heap.begin(), heap.end(), compare_dt());
		heap.pop_back();

		calc_cost_dt(t, tiles[t.y - 1][t.x + 0]);
		calc_cost_dt(t, tiles[t.y + 1][t.x + 0]);

		calc_cost_dt(t, tiles[t.y + 0][t.x - 1]);
		calc_cost_dt(t, tiles[t.y + 0][t.x + 1]);

		calc_cost_dt(t, tiles[t.y + 1][t.x + 1]);
		calc_cost_dt(t, tiles[t.y - 1][t.x - 1]);

		calc_cost_dt(t, tiles[t.y - 1][t.x + 1]);
		calc_cost_dt(t, tiles[t.y + 1][t.x - 1]);

		t.vdt = false;
	}
}

static void constexpr
calc_cost_d(tile const &a, tile &b)
{
	if (b.vd && b.d > a.d) {
		b.d = a.d + 1;
	}
}

static void constexpr
calc_cost_dt(tile const &a, tile &b)
{
	if (b.vdt && b.dt > a.dt + a.h/TUNNEL_STRENGTH) {
		b.dt = a.dt + 1 + a.h/TUNNEL_STRENGTH;
	}
}
