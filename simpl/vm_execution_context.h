#ifndef __simpl_vm_execution_context_h__
#define __simpl_vm_execution_context_h__

#include "expression.h"
#include "statement.h"
#include "vm.h"
#include <functional>

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
			vm_.reg_fn("print", 1, [this]()
			{
				std::cout << to_string(vm_.stack_offset(0));
			});
			vm_.reg_fn("println", 1, [this]()
			{
				std::cout << to_string(vm_.stack_offset(0)) << "\n";
			});
		}

	public:
		virtual void visit(call_statement &cs)
		{
			// evaluating the expression puts it on the stack.
			for(const auto &expr : cs.expr())
				expr->evaluate(*this);
			// we also need to put a return value placeholder, if there's a retval
			vm_.call(cs.function());
			vm_.decrement_stack(cs.expr().size());
		}

		virtual void visit(let_statement &cs)
		{
			if (cs.expr())
			{
				cs.expr()->evaluate(*this);
			}

			vm_.create_var(cs.name());
		}

		virtual void visit(if_statement &is)
		{
			if (is.cond())
				is.cond()->evaluate(*this);
			const auto &val = vm_.pop_stack();
			if (is_true(val))
			{
				scope s{ vm_ };
				is.statement()->evaluate(*this);
			}
		}

		virtual void visit(def_statement &ds)
		{
			auto name = ds.name();
			auto arity = ds.identifiers().size();
			auto stmt = std::move(ds.release_statement());
			fn_def fn
			{ 
				name,
				arity,
				[this, arity, ids=ds.identifiers(), stmt=std::shared_ptr<statement>(stmt.release())]() 
				{
					int offset = arity - 1;
					scope s{vm_};
					for (; offset >= 0; --offset)
						vm_.create_var(ids[ids.size()-(offset+1)].name, offset);
					stmt->evaluate(*this);					
				}
			};
			vm_.reg_fn(std::move(fn));
		}

		virtual void visit(assignment_statement &as)
		{
			if (as.expr())
				as.expr()->evaluate(*this); // places value on stack.
			vm_.set_val(as.identifier(), 0);
			vm_.pop_stack(); // take the value off the top.
		}

		virtual void visit(while_statement &ws)
		{
			
			// don't mind this little goto trick...
			run_cond:
			ws.cond()->evaluate(*this);
			const auto &val = vm_.pop_stack();
			if (!is_true(val))
				return;
			scope s{ vm_ };
			ws.block()->evaluate(*this);
			goto run_cond;
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
				vm_.push_stack(vm_.load_var(id.name));
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
			case op_type::eqeq:
			{
				do_eqeq(cs, vm_);
				break;
			}
			case op_type::neq:
			{
				do_neq(cs, vm_);
				break;
			}
			case op_type::lt:
			{
				do_lt(cs, vm_);
				break;
			}
			case op_type::lteq:
			{
				do_lteq(cs, vm_);
				break;
			}
			case op_type::gt:
			{
				do_gt(cs, vm_);
				break;
			}
			case op_type::gteq:
			{
				do_gteq(cs, vm_);
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
		void do_eqeq(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid equality");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(is_equal(lvalue, rvalue));

		}
		void do_neq(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid equality");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(!is_equal(lvalue, rvalue));
		}
		void do_lt(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid equality");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(is_lt(lvalue,rvalue));
		}
		void do_lteq(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid equality");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(is_lteq(lvalue,rvalue));
		}
		void do_gt(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid equality");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(is_gt(lvalue,rvalue));
		}
		void do_gteq(nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("invalid equality");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(is_gteq(lvalue,rvalue));
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
		int is_equal(const value_t &left, const value_t &right)
		{
			if (left.index() != right.index())
				return 0;
			if (left.index() == 1)
			{
				return std::get<int>(left) == std::get<int>(right) ? 1 : 0;
			}
			else
				return std::get<std::string>(left) == std::get<std::string>(right) ? 1 : 0;
			return 0;
		}
		int is_lt(const value_t &left, const value_t &right)
		{
			if (left.index() != right.index())
				return 0;
			if (left.index() == 1)
			{
				return std::get<int>(left) < std::get<int>(right) ? 1 : 0;
			}
			else
				return std::get<std::string>(left) < std::get<std::string>(right) ? 1 : 0;
			return 0;
		}
		int is_lteq(const value_t &left, const value_t &right)
		{
			if (left.index() != right.index())
				return 0;
			if (left.index() == 1)
			{
				return std::get<int>(left) <= std::get<int>(right) ? 1 : 0;
			}
			else
				return std::get<std::string>(left) <= std::get<std::string>(right) ? 1 : 0;
			return 0;
		}
		int is_gt(const value_t &left, const value_t &right)
		{
			if (left.index() != right.index())
				return 0;
			if (left.index() == 1)
			{
				return std::get<int>(left) > std::get<int>(right) ? 1 : 0;
			}
			else
				return std::get<std::string>(left) > std::get<std::string>(right) ? 1 : 0;
			return 0;
		}
		int is_gteq(const value_t &left, const value_t &right)
		{
			if (left.index() != right.index())
				return 0;
			if (left.index() == 1)
			{
				return std::get<int>(left) >= std::get<int>(right) ? 1 : 0;
			}
			else
				return std::get<std::string>(left) >= std::get<std::string>(right) ? 1 : 0;
			return 0;
		}

	public:
		vm &vm_;

	};
}

#endif // __simpl_vm_execution_context_h__