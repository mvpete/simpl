#ifndef __simpl_vm_execution_context_h__
#define __simpl_vm_execution_context_h__

#include "expression.h"
#include "statement.h"
#include "vm.h"

namespace simpl
{
	class scope
	{
	public:
		scope(vm &vm)
			:vm_(vm)
		{
			vm_.enter_scope();
		}
		~scope()
		{
			vm_.exit_scope();
		}
	private:
		vm &vm_;
	};
	class vm_execution_context : public statement_visitor, public expression_visitor
	{
	public:
		vm_execution_context(simpl::vm &vm)
			:vm_(vm)
		{
			vm_.reg_fn("print", [](simpl::vm &vm)
			{
				const auto top = vm.pop_stack();
				std::cout << to_string(top);
			});
			vm_.reg_fn("println", [](simpl::vm &vm)
			{
				const auto top = vm.pop_stack();
				std::cout << to_string(top) << "\n";
			});
		}

	public:
		virtual void visit(call_statement &cs)
		{
			if (cs.expr())
				cs.expr()->evaluate(*this);
			vm_.call(cs.function());
		}

		virtual void visit(let_statement &cs)
		{
			value_t v;
			if (cs.expr())
			{
				cs.expr()->evaluate(*this);
				v = vm_.pop_stack();
			}

			vm_.create_var(cs.name(), v);
		}

		virtual void visit(if_statement &is)
		{
			if (is.cond())
				is.cond()->evaluate(*this);
			const auto val = vm_.pop_stack();
			if (is_true(val))
			{
				scope s{ vm_ };
				is.statement()->evaluate(*this);
			}
		}

		virtual void visit(expression &ex)
		{
			if (ex.value().index() == 0)
				throw  std::logic_error("invalid expression");
			if (ex.value().index() == 1)
			{
				vm_.push_stack(std::get<value_t>(ex.value()));
			}
			else if (ex.value().index() == 2)
			{
				std::get<expression_ptr>(ex.value())->evaluate(*this);
			}
			else if (ex.value().index() == 3)
			{
				auto id = std::get<identifier>(ex.value());
				vm_.load_var(id.name);
			}
		}

		virtual void visit(nary_expression &cs)
		{
			switch (cs.op())
			{
			case op_type::add:
			{
				do_add(cs, vm_);
				break;
			}
			case op_type::sub:
			{
				do_sub(cs, vm_);
				break;
			}
			case op_type::div:
			{
				do_div(cs, vm_);
				break;
			}
			case op_type::mult:
			{
				do_mult(cs, vm_);
				break;
			}
			}
		}

	private:
		void do_add(const nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid add");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) + std::get<int>(rvalue));
		}
		void do_sub(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid add");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) - std::get<int>(rvalue));
		}
		void do_div(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid add");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) / std::get<int>(rvalue));
		}
		void do_mult(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid add");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) * std::get<int>(rvalue));
		}

		bool is_true(const value_t &v)
		{
			if (v.index() == 0)
				return false;
			if (v.index() == 1)
			{
				return std::get<int>(v) > 0;
			}
			else if (v.index() == 2)
			{
				const auto &strval = std::get<std::string>(v);
				return strval == "true" || strval == "1" || strval == "t" || strval == "TRUE" || strval == "T";
			}
			return false;
		}

	public:
		vm &vm_;

	};
}

#endif // __simpl_vm_execution_context_h__