#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <cstdint>
// #include <optional>
#include <vector>
#include <string>
#include <random>
#include <limits>
#include <cstdlib>
namespace skiplist
{
	using key_type = uint64_t;
	// using value_type = std::vector<char>;
	using value_type = std::string;
	struct skiplistNode
	{
		key_type key;
		value_type value;
		skiplistNode **forward;
		skiplistNode(int level, key_type k, const value_type &v) : key(k), value(v)
		{
			forward = new skiplistNode *[level + 1](); // notice level + 1 !!!
		}
		~skiplistNode()
		{
			delete[] forward;
		}
	};
	class skiplist_type
	{
		// add something here
	private:
		skiplistNode *header;
		double p;
		int maxLevel;
		int currentLevel;
		int randomLevel();

	public:
		explicit skiplist_type(double p = 0.5);
		~skiplist_type();
		void put(key_type key, const value_type &val);
		// std::optional<value_type> get(key_type key) const;
		std::string get(key_type key) const;

		// for hw1 only
		int query_distance(key_type key) const;
	};
} // namespace skiplist
#endif // SKIPLIST_H
