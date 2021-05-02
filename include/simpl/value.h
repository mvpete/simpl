#ifndef __simpl_value_h__
#define __simpl_value_h__

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace simpl
{
	struct empty_t {};
	struct blob_t;
	struct array_t;
	
	using blobref_t = std::shared_ptr<blob_t>;
	blobref_t new_blob()
	{
		return std::make_shared<blob_t>();
	}

	using arrayref_t = std::shared_ptr<array_t>;
	arrayref_t new_array()
	{
		return std::make_shared<array_t>();
	}

	using value_t = std::variant<empty_t, bool, double, std::string, blobref_t, arrayref_t>;
	struct blob_t { std::map<std::string, value_t> values; };
	struct array_t { std::vector<value_t> values; };



}

#endif //__simpl_value_h__