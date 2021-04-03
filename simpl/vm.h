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
		std::map<std::string, value_t> variables_;
		std::map<std::string, std::function<void(vm&)>> functions_;
		std::stack<value_t> stack_;
	public:

		vm()
		{
			functions_["print"] = [](vm &vm) 
			{				
				const auto top = vm.pop_stack();
				std::cout << to_string(top);
			};
		}

		void call(const std::string &fn_s)
		{
			auto fn = functions_.find(fn_s);
			if (fn != functions_.end())
			{
				fn->second(*this);
			}
			else
				throw std::runtime_error("function not defined");
		}

		void push_stack(const value_t &v)
		{
			stack_.push(v);
		}

		value_t pop_stack()
		{
			if (stack_.empty())
			{
				throw std::runtime_error("stack is empty");
			}
			const auto top = stack_.top();
			stack_.pop();
			return top;
		}

		template <typename CallableT>
		void reg_fn(const std::string &id, CallableT &&fn)
		{
			functions_[id] = fn;
		}

		void create_var(const std::string &name, value_t &value)
		{
			if (variables_.find(name) != variables_.end())
				throw std::runtime_error("already defined");
			variables_[name] = value;
		}
		void store_var(const std::string &name, value_t &value)
		{
			if (variables_.find(name) == variables_.end())
				throw std::runtime_error("undefined variable");
			variables_[name] = value;
		}
		void load_var(const std::string &name)
		{
			if (variables_.find(name) == variables_.end())
			{
				std::stringstream ss;
				ss << "undefined variable '" << name << "'";
				throw std::runtime_error(ss.str());
			}
			stack_.push(variables_[name]);
		}

	};

	/*class vm_statement_visitor : public statement_visitor
	{
	public:
		vm_statement_visitor(vm &vm)
			:vm_(vm)
		{
		}

	public:
		virtual void visit(call_statement &cs)
		{

		}

		virtual void visit(let_statement &cs)
		{
		}

		virtual void visit(if_statement &is)
		{
		}

		virtual void visit(block_statement &bs)
		{
		}

	public:
		vm &vm_;

	};*/

}


#endif //__simpl_vm_h__