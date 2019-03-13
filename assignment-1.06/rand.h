#include <random>
#include <string>
#include <type_traits>

class ranged_random {
private:
	std::mt19937 gen;
public:
	long unsigned int seed;

	ranged_random()
	{
		seed = std::random_device{}();
		gen.seed(seed);
	}

	explicit ranged_random(std::string const &s)
	{
		long unsigned int hash = 5381;

		for(auto const c : s) {
			hash = ((hash << 5)) ^ c;
		}

		seed = hash;
		gen.seed(seed);
	}

	explicit ranged_random(long unsigned int const s)
	{
		seed = s;
		gen.seed(seed);
	}

	template<typename T> T
	rrand(T a, T b)
	{
		static_assert(std::is_integral<T>::value,
			"integral type required");

		std::uniform_int_distribution<T> dis(a, b);

		return dis(gen);
	}
};
