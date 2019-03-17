#include <random>
#include <string>

class ranged_random {
private:
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
};
