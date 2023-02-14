#ifndef __simpl_array_h__
#define __simpl_array_h__

#include <simpl/library.h>

#include <algorithm>
#include <random>

namespace simpl
{

	template <typename Iterator>
	using iterator_value_t = typename std::iterator_traits<Iterator>::value_type;



	template <typename IteratorT>
	std::vector<iterator_value_t<IteratorT>> subset(IteratorT begin, IteratorT end)
	{
		return std::vector<iterator_value_t<IteratorT>> { begin,end };
	}

	class array_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "array";
		}

		void load(vm &vm) override
		{
			vm.reg_fn("size", [](const array_t &arr)
			{
				return (double)arr.values.size();
			});

			vm.reg_fn("push", [](array_t &arr, const value_t &v)
			{
				arr.values.push_back(v);
			});

			vm.reg_fn("pop", [](array_t &arr)
			{
				arr.values.pop_back();
			});

			vm.reg_fn("shuffle", [](array_t &arr)
			{
				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(arr.values.begin(), arr.values.end(), g);
			});
			vm.reg_fn("slice", [](array_t &arr, double cnt)
			{
				return make_array(subset(arr.values.begin() + static_cast<int>(cnt), arr.values.end()));
			});
			vm.reg_fn("take", [](array_t &arr, double cnt)
			{
				return make_array(subset(arr.values.begin(), arr.values.begin() + static_cast<int>(cnt)));
			});

		}
	};
}



#endif // __simpl_array_h__