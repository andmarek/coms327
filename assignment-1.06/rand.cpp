#include "rand.h"

ranged_random::ranged_random()
{
	seed = std::random_device{}();
	gen.seed(seed);
}

ranged_random::ranged_random(std::string const &s)
{
	long unsigned int hash = 5381;

	for(auto const c : s) {
		hash = ((hash << 5)) ^ c;
	}

	seed = hash;
	gen.seed(seed);
}

ranged_random::ranged_random(long unsigned int const s)
{
	seed = s;
	gen.seed(seed);
}
