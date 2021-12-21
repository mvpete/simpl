#ifndef __simpl_vm_execution_context_h__
#define __simpl_vm_execution_context_h__

#include <simpl/expression.h>
#include <simpl/operations.h>
#include <simpl/statement.h>
#include <simpl/vm.h>

#include <functional>

namespace simpl
{

	namespace detail
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

		std::string to_simpl_type_string(vm &vm, const simpl::argument &a)
		{
			const auto &simpl_type = a.type;
			auto type_str = simpl_type.has_value() ? detail::to_builtin_type_string(simpl_type.value()) : "any";
			if (!type_str.has_value() && vm.has_type(simpl_type.value()))
				type_str = vm.lookup_type(simpl_type.value())->name;
			if (!type_str.has_value())
				throw std::runtime_error(detail::format("unknown type {0}", (simpl_type.has_value() ? simpl_type.value() : "<t>")));
			return type_str.value();
		}

		std::string format_name(vm& vm, const std::string &name, const std::vector<argument> &arguments)
		{
			std::stringstream ss;
			ss << name << "(";
			for (size_t i = 0; i < arguments.size(); ++i)
			{
				const auto type_str = to_simpl_type_string(vm, arguments[i]);
				ss << type_str;
				if (i != arguments.size() - 1)
					ss << ",";
			}
			ss << ")";
			return ss.str();
		}

		std::vector<std::string> to_arg_types(vm &vm, const std::vector<simpl::argument> &args)
		{
			std::vector<std::string> ret;
			for (const simpl::argument &a : args)
			{
				ret.push_back(to_simpl_type_string(vm,a));
			}
			return ret;
		}
	}

	class vm_execution_context : public statement_visitor, public expression_visitor
	{
	public:
		vm_execution_context(simpl::vm &vm)
			:vm_(vm)
		{
			vm_.reg_fn("is_empty", [](const value_t &v)
			{
				return std::holds_alternative<empty_t>(v);
			});
		}

	public:
		virtual void visit(expr_statement &cs)
		{
			if (cs.expr())
				cs.expr()->evaluate(*this);
			vm_.pop_stack(); // an expression leaves a value on the top of the stack.			
		}

		virtual void visit(object_definition_statement &os)
		{
			vm_.register_type(os.type_name(), os.inherits(), os.move_members());
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
			auto if_stmt = &is;
			while (if_stmt != nullptr)
			{
				if (if_stmt->cond())
					if_stmt->cond()->evaluate(*this);
				const auto &val = vm_.pop_stack();
				if (is_true(val))
				{
					detail::scope s{ vm_ };
					if_stmt->statement()->evaluate(*this);
					break;
				}

				if (if_stmt->next()) // else if
				{
					if_stmt = if_stmt->next().get();
					continue;
				}
				else if (if_stmt->else_statement()) // else
				{
					if_stmt->else_statement()->evaluate(*this);
					break;
				}
				else
					break; // no else
			}
		}

		virtual void visit(def_statement &ds)
		{
			auto id = detail::format_name(vm_, ds.name(), ds.arguments());
			auto arity = ds.arguments().size();
			auto stmt = std::move(ds.release_statement());
			detail::fn_def fn
			{
				id,
				ds.name(),
				detail::to_arg_types(vm_, ds.arguments()),
				[this, arity, ids = ds.arguments(), stmt = std::shared_ptr<statement>(stmt.release())]()
				{
					int offset = arity - 1;
					for (; offset >= 0; --offset)
						vm_.create_var(ids[ids.size() - (offset + 1)].name, offset);
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
				if (depth > vm_.depth())
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
			detail::scope s{ vm_ };
			ws.block()->evaluate(*this);
			goto run_cond;
		}

		virtual void visit(for_statement &fs)
		{
			// we need to add the loop scope.
			detail::scope loop{ vm_ };
			fs.init()->evaluate(*this);
		run_for_cond:
			fs.cond()->evaluate(*this);
			const auto &val = vm_.pop_stack();
			if (!is_true(val))
				return;
			{
				detail::scope s{ vm_ };
				fs.block()->evaluate(*this);
			}
			fs.incr()->evaluate(*this);
			goto run_for_cond;
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
				load_identifier(std::get<identifier>(ex.value()));
			}
		}

		virtual void visit(new_blob_expression &ns)
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

		virtual void visit(new_array_expression &nas)
		{
			auto array = new_array();
			vm_.push_stack(array);
			for (const auto &expr : nas.expressions())
			{
				expr->evaluate(*this);
				array->values.push_back(vm_.pop_stack());
			}
		}

		virtual void visit(new_object_expression &nos)
		{
			// lookup the type
			auto type = vm_.lookup_type(nos.type());

			if (type == nullptr)
				throw std::runtime_error("unknown type");


			auto object = new_simpl_object(nos.type());
			vm_.push_stack(object);

			// initialize the members based on hierarchy

			// run the type initializers

			// run the expression initializers


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
			case op_type::log_and:
			{
				do_and(cs, vm_);
				break;
			}
			case op_type::log_or:
			{
				do_or(cs, vm_);
				break;
			}
			case op_type::func:
			{
				do_func(cs, vm_);
				break;
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

		
		void do_and(const nary_expression &exp, vm &vm)
		{
			if(exp.expressions().size() != 2)
				throw std::logic_error("bad arity");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			if (!is_true(vm_.stack_offset(0)))
				return;
			vm_.pop_stack();
			right->evaluate(*this);
		}

		void do_or(const nary_expression &exp, vm &vm)
		{
			if (exp.expressions().size() != 2)
				throw std::logic_error("bad arity");
			auto &left = exp.expressions()[1];
			auto &right = exp.expressions()[0];

			left->evaluate(*this);
			if (is_true(vm_.stack_offset(0)))
				return;
			vm_.pop_stack();
			right->evaluate(*this);
		}
		
		void do_func(nary_expression &exp, vm &vm)
		{
			vm_.push_stack(value_t{}); // we put an empty value on the stack -- this is the retval;
			// because all expressions, leave a value on the stack.
			// evaluating the expression puts it on the stack.
			for (const auto &expr : exp.expressions())
				expr->evaluate(*this);			
			call_def cd{ exp.function(), make_arg_list(vm, exp.expressions().size()) };
			vm_.call(cd);
			vm_.decrement_stack(exp.expressions().size());
		}
 
		std::vector<std::string> make_arg_list(vm& vm, size_t s)
		{
			std::vector<std::string> args;
			for (size_t i = s; i > 0; --i)
			{
				args.push_back(detail::get_type_string(vm.stack_offset(i-1)));
			}
			return args;
		}

		bool is_true(const value_t &v)
		{
			return cast<bool>(v);
		}

		void load_identifier(const identifier &id)
		{
			auto &val = vm_.load_var(id.name);
			if (id.path.size() == 0)
			{
				vm_.push_stack(val);
			}
			else
			{
				auto next_val = val;
				for (const auto &indexor : id.path)
				{
					next_val = at(next_val, indexor);
				}
				vm_.push_stack(next_val);
			}
		}

		value_t &at(value_t &val, indexor at)
		{
			if (std::holds_alternative<size_t>(at))
			{
				// check that the variable is an array
				if (!std::holds_alternative<arrayref_t>(val))
					throw std::runtime_error("not an array");

				auto array = std::get<arrayref_t>(val);
				return array->values.at(std::get<size_t>(at));
			}

			auto &name = std::get<std::string>(at);

			// if the item is in scope we'll use that first.
			if (vm_.in_scope(name))
			{
				int idx = (int)cast<double>(vm_.load_var(name));

				if (!std::holds_alternative<arrayref_t>(val))
					throw std::runtime_error("not an array");

				auto array = std::get<arrayref_t>(val);
				return array->values.at(idx);
			}

			// check that the variable is a blob, and see if the value is a 
			// key on him.
			if (!std::holds_alternative<blobref_t>(val))
				throw std::runtime_error("not an object");

			auto blob = std::get<blobref_t>(val);
			return blob->values.at(name);

		}
		

	public:
		vm &vm_;

	};
}

#endif // __simpl_vm_execution_context_h__