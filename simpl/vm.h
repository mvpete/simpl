#ifndef __simpl_vm_h__
#define __simpl_vm_h__

#include "value.h"

#include <iostream>
#include <functional>
#include <map>
#include <stack>

namespace simpl
{
	class vm
	{
		std::map<std::string, std::function<void()>> functions_;
		std::stack<value_t> stack_;
	public:

		vm()
		{
			functions_["print"] = [this]() 
			{
				if (stack_.empty())
				{
					throw std::runtime_error("argument error");
				}
				const auto top = pop_stack();
				std::cout << to_string(top);
			};
		}

		void call(const std::string &fn_s)
		{
			auto fn = functions_.find(fn_s);
			if (fn != functions_.end())
			{
				fn->second();
			}
		}

		void push_stack(const value_t &v)
		{
			stack_.push(v);
		}

		value_t pop_stack()
		{
			const auto top = stack_.top();
			stack_.pop();
			return top;
		}
	};
}


#endif //__simpl_vm_h__