#ifndef RAND_H
#define RAND_H

#include <random>
#include <string>

class ranged_random {
	std::mt19937 gen;
public:
	long unsigned int seed;

	ranged_random();

	explicit ranged_random(std::string const &);

	explicit ranged_random(long unsigned int const);

	template<typename T> T
	rrand(T a, T b)
	{
		std::uniform_int_distribution<T> dis(a, b);

		return dis(gen);
	}

	template<typename T> T
	rand_dice(T base, T dice, T sides)
	{
		T total = base;
		for (size_t i = 0; i < dice; ++i) {
			total = static_cast<T>(total + rrand<T>(1, sides));
		}
		return total;
	}
};

#endif /* RAND_H */
