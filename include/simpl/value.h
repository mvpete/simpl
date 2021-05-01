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

	std::string get_type_string(const value_t &v)
	{
		if (std::holds_alternative<empty_t>(v))
			return typeid(empty_t{}).name();
		else if (std::holds_alternative<bool>(v))
			return typeid(bool{}).name();
		else if (std::holds_alternative<double>(v))
			return typeid(double{}).name();
		else if (std::holds_alternative<std::string>(v))
			return typeid(std::string{}).name();
		else if (std::holds_alternative<blobref_t>(v))
			return typeid(blob_t{}).name();
		else if (std::holds_alternative<arrayref_t>(v))
			return typeid(array_t{}).name();
		throw std::runtime_error("unknown type");
	}

}

#endif //__simpl_value_h__