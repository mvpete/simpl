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
		class var_scope
		{
			vm &vm_;
			std::vector<std::string> vars_;
		public:
			var_scope(vm &vm)
				:vm_(vm)
			{
			}
			~var_scope()
			{
				for (const auto &v : vars_)
					vm_.clear_var(v);
			}
			void track(const std::string &name)
			{
				vars_.push_back(name);
			}

		};

		std::map<std::string, value_t> variables_;
		std::stack<var_scope> scope_;
		std::map<std::string, std::function<void(vm&)>> functions_;
		std::stack<value_t> stack_;
	public:

		vm()
		{
			scope_.push(var_scope{*this});
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
			scope_.top().track(name);
		}
		void store_var(const std::string &name, value_t &value)
		{
			if (variables_.find(name) == variables_.end())
				throw std::runtime_error("undefined variable");
			variables_[name] = value;
		}
		void clear_var(const std::string &name)
		{
			auto v = variables_.find(name);
			if (v == variables_.end())
				throw std::runtime_error("undefined variable");
			variables_.erase(v);
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

		void enter_scope()
		{
			scope_.push(var_scope{ *this });
		}

		void exit_scope()
		{
			if(!scope_.empty())
				scope_.pop();
		}

	};



}


#endif //__simpl_vm_h__