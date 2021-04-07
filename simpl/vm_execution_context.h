#ifndef __simpl_vm_execution_context_h__
#define __simpl_vm_execution_context_h__

#include "expression.h"
#include "operations.h"
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
		void reg_var(const std::string &name, size_t offset)
		{
			vm_.create_var(name, offset);
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
				std::cout << cast<std::string>(vm_.stack_offset(0));
			});
			vm_.reg_fn("println", 1, [this]()
			{
				std::cout << cast<std::string>(vm_.stack_offset(0)) << "\n";
			});
		}

	public:
		virtual void visit(call_statement &cs)
		{
			if (cs.expr())
				cs.expr()->evaluate(*this);
			vm_.pop_stack(); // an expression leaves a value on the top of the stack.			
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
					for (; offset >= 0; --offset)
						vm_.create_var(ids[ids.size()-(offset+1)].name, offset);
					stmt->evaluate(*this);					
				}
			};
			vm_.reg_fn(std::move(fn));
		}

		virtual void visit(return_statement &rs)
		{
			if (rs.expr())
				rs.expr()->evaluate(*this);
			else
				vm_.push_stack(value_t{});
			// then the value is at the top of the stack.
			// now, put it into the activation record location
			vm_.return_();
			vm_.pop_stack();
		}

		virtual void visit(assignment_statement &as)
		{
			if (as.expr())
				as.expr()->evaluate(*this); // places value on stack.
			vm_.set_val(as.identifier(), 0); // set the value of the identifier, to the value at the top.
			vm_.pop_stack(); // take the value off the top.
		}

		virtual void visit(block_statement &bs)
		{
			// we need to capture the current activation ctx.
			// if it changes, we're done. I think.
			const auto depth = vm_.depth();
			for (const auto &stmt : bs.statements())
			{
				stmt->evaluate(*this);
				if(depth > vm_.depth())
					return;
			}
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
			if (std::holds_alternative<empty_t>(ex.value()))
				throw  std::logic_error("invalid expression");

			if (std::holds_alternative<value_t>(ex.value()))
			{
				vm_.push_stack(std::get<value_t>(ex.value()));
			}
			else if (std::holds_alternative<expression_ptr>(ex.value()))
			{
				std::get<expression_ptr>(ex.value())->evaluate(*this);
			}
			else if (std::holds_alternative<identifier>(ex.value()))
			{
				auto id = std::get<identifier>(ex.value());
				vm_.push_stack(vm_.load_var(id.name));
			}
		}

		virtual void visit(new_expression &ns)
		{
			auto blob = new_blob();
			vm_.push_stack(blob);
			for (const auto &init : ns.initializers())
			{
				if (init.expr)
				{
					init.expr->evaluate(*this);
					blob->values[init.identifier] = vm_.pop_stack();
				}
			}
		}

		virtual void visit(nary_expression &cs)
		{
			switch (cs.op())
			{
			case op_type::add:
			{
				do_binary<add_op>(cs, vm_);
				break;
			}
			case op_type::sub:
			{
				do_binary<sub_op>(cs, vm_);
				break;
			}
			case op_type::div:
			{
				do_binary<div_op>(cs, vm_);
				break;
			}
			case op_type::mult:
			{
				do_binary<mult_op>(cs, vm_);
				break;
			}
			case op_type::eqeq:
			{
				do_binary<eqeq_op>(cs, vm_);
				break;
			}
			case op_type::neq:
			{
				do_binary<neq_op>(cs, vm_);
				break;
			}
			case op_type::lt:
			{
				do_binary<lt_op>(cs, vm_);
				break;
			}
			case op_type::lteq:
			{
				do_binary<lte_op>(cs, vm_);
				break;
			}
			case op_type::gt:
			{
				do_binary<gt_op>(cs, vm_);
				break;
			}
			case op_type::gteq:
			{
				do_binary<gte_op>(cs, vm_);
				break;
			}
			case op_type::func:
			{
				do_func(cs, vm_);
			}
			}
		}

	private:

		template <typename OpT>
		void do_binary(const nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("bad arity");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			right->evaluate(*this);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(apply<OpT>(lvalue, rvalue));
		}
		
		void do_func(nary_expression &exp, vm &vm)
		{
			vm_.push_stack(value_t{}); // we put an empty value on the stack -- this is the retval;
			// because all expressions, leave a value on the stack.
			// evaluating the expression puts it on the stack.
			for (const auto &expr : exp.expressions())
				expr->evaluate(*this);
			scope s{ vm_ };
			vm_.call(exp.function());
			vm_.decrement_stack(exp.expressions().size());
		}

		bool is_true(const value_t &v)
		{
			return cast<bool>(v);
		}
		

	public:
		vm &vm_;

	};
}

#endif // __simpl_vm_execution_context_h__