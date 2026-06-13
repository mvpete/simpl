#ifndef __simpl_vm_execution_context_h__
#define __simpl_vm_execution_context_h__

#include <simpl/expression.h>
#include <simpl/operations.h>
#include <simpl/parser.h>
#include <simpl/statement.h>
#include <simpl/vm.h>

#include <cstdlib>
#include <fstream>
#include <functional>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

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
		private:
			vm &vm_;
		};

		inline std::string to_simpl_type_string(vm &vm, const simpl::argument &a)
		{
			const auto &simpl_type = a.type;
			auto type_str = simpl_type.has_value() ? detail::to_builtin_type_string(simpl_type.value()) : "any";
			if (!type_str.has_value() && vm.has_type(simpl_type.value()))
				type_str = vm.lookup_type(simpl_type.value())->name;
			if (!type_str.has_value())
				throw std::runtime_error(detail::format("unknown type '{0}'", (simpl_type.has_value() ? simpl_type.value() : "<t>")));
			return type_str.value();
		}

		inline std::string format_name(vm& vm, const std::string &name, const std::vector<argument> &arguments)
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

		inline std::vector<std::string> to_arg_types(vm &vm, const std::vector<simpl::argument> &args)
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
			vm_.register_type<simpl::value>("any");
			vm_.register_type<simpl::empty>("empty");
			vm_.register_type<simpl::string>("string");
			vm_.register_type<simpl::boolean>("bool");
			vm_.register_type<simpl::number>("number");
			vm_.register_type<simpl::blob>("blob");
			vm_.register_type<simpl::array>("array");

			vm_.reg_fn("is_empty", [](const value_t &v)
			{
				return std::holds_alternative<empty_t>(v);
			});
		}

	public:
		virtual void visit(expr_statement &cs)
		{
			auto sz = vm_.stack_size();
			if (cs.expr())
				cs.expr()->evaluate(*this);
			vm_.decrement_stack(vm_.stack_size() - sz); // an expression leaves values on the top of the stack.			
		}

		virtual void visit(object_definition_statement &os)
		{
			vm_.register_type(os.type_name(), os.inherits(), os.move_members());
		}

		virtual void visit(let_statement &cs)
		{
			auto sz = vm_.stack_size();
			if (cs.expr())
			{
				cs.expr()->evaluate(*this);
			}
			else
			{
				vm_.push_stack(value_t{}); // empty value.
			}
			if (vm_.stack_size() - sz > 1)
				throw std::runtime_error("invalid expression.");

			vm_.create_local_var(cs.name());
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
			auto stmt = ds.release_statement();
			detail::fn_def fn
			{
				id,
				ds.name(),
				detail::to_arg_types(vm_, ds.arguments()),
				[this, arity, ids = ds.arguments(), stmt = std::shared_ptr<statement>(stmt.release())]()
				{
					int offset = arity - 1;
					for (; offset >= 0; --offset)
						vm_.track_stack_var(ids[ids.size() - (offset + 1)].name, offset);
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
			{
				detail::scope s{ vm_ };
				ws.block()->evaluate(*this);
			}
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

		virtual void visit(import_statement& is) override
		{
			auto done = std::find(imported_.begin(), imported_.end(), is.libname());
			if (done != imported_.end())
				return; // already imported.

			auto ctx = std::find(importing_.begin(), importing_.end(), is.libname());
			if (ctx != importing_.end())
				throw std::runtime_error("cyclical import detected.");
			importing_.emplace_back(is.libname());

			std::filesystem::path p(is.libname());
			auto ext = p.extension().string();

			bool loaded = false;
			if (ext.empty())
			{
				// No extension: look up a pre-registered built-in library by name.
				loaded = vm_.load_library(is.libname());
			}
			else if (ext == ".sl")
			{
				loaded = import_sl(is.libname());
			}
			else if (ext == ".dll" || ext == ".so" || ext == ".dylib")
			{
				loaded = import_native(is.libname());
			}
			else
			{
				throw std::runtime_error(detail::format("unknown module extension '{0}'.", ext));
			}

			if (!loaded)
				throw std::runtime_error(detail::format("module '{0}' not found.", is.libname()));

			imported_.emplace_back(is.libname());
			importing_.pop_back();
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

			// Initialize the members based on hierarchy
			// TODO: Optimize this based on each levels initializers.
			// TODO: Also, really need to look at identifiers and whether 
			// they're defined or not. But, this could be done in the 
			// parser, if the parser knows about the types.
			std::stack<const detail::type_def *> types;
			while (type != nullptr)
			{
				types.push(type);
				type = type->inherits;
			}
			// run the type initializers
			while (types.size() > 0)
			{
				type = types.top();
				for (const auto &mi : type->members)
				{
					// Again -- this could be a parse time thing...
					if (object->members.find(mi.name) != object->members.end() && mi.initializer == nullptr)
						throw std::runtime_error(detail::format("redfinition of member '{0}' in type '{1}'", mi.name, nos.type()));

					if (mi.initializer)
					{
						mi.initializer->evaluate(*this);
						object->members[mi.name] = vm_.pop_stack();
					}
					else
					{
						object->members[mi.name] = value_t{};
					}
				}
				types.pop();
			}

			// run the expression initializers
			for (const auto &init : nos.initializers())
			{
				if (init.expr)
				{
					init.expr->evaluate(*this);
					object->members[init.identifier] = vm_.pop_stack();
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
			case op_type::eq:
			{
				do_assignment(cs, vm_);
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
			case op_type::expand:
			{
				do_expand(cs, vm_);
				break;
			}
			case op_type::increment:
			{
				do_increment(cs, vm_);
				break;
			}
			case op_type::decrement:
			{
				do_decrement(cs, vm_);
				break;
			}
			default:
				throw std::runtime_error("invalid operation.");
			}
		}

		virtual void visit(function_address_expression& fae)
		{
			// For now, we just push the function name on the stack.
			vm_.push_stack(fae.name());
		}

		void evaluate(syntax_tree& ast)
		{
			for (auto& stmt : ast)
			{
				evaluate(std::move(stmt));
			}
		}

		void evaluate(statement_ptr statement)
		{
			statement->evaluate(*this);
		}

	private:

		// Returns the directories to search, in the 5-step order:
		//   1. Current working directory
		//   2. Directory of the currently executing script (if any)
		//   3. Directories from the SIMPL_PATH environment variable
		//   4. Directory of the simpl executable
		//   5. Directories from the PATH environment variable
		std::vector<std::filesystem::path> make_search_path() const
		{
			std::vector<std::filesystem::path> paths;

			// Step 1: current working directory.
			paths.push_back(std::filesystem::current_path());

			// Step 2: directory of the currently executing script.
			if (!script_dirs_.empty())
				paths.push_back(script_dirs_.back());

			// Steps 3 & 5: split an environment variable on the platform path separator.
			auto split_env = [&](const char* var)
			{
#ifdef _WIN32
				char* env = nullptr;
				size_t len = 0;
				if (_dupenv_s(&env, &len, var) != 0 || env == nullptr)
					return;
				std::string s(env);
				free(env);
#else
				const char* env = std::getenv(var);
				if (!env) return;
				std::string s(env);
#endif
#ifdef _WIN32
				const char sep = ';';
#else
				const char sep = ':';
#endif
				std::stringstream ss(s);
				std::string dir;
				while (std::getline(ss, dir, sep))
					if (!dir.empty())
						paths.push_back(dir);
			};

			// Step 3: SIMPL_PATH environment variable.
			split_env("SIMPL_PATH");

			// Step 4: directory of the simpl executable.
			auto exe_dir = get_executable_directory();
			if (!exe_dir.empty())
				paths.push_back(exe_dir);

			// Step 5: PATH environment variable.
			split_env("PATH");

			return paths;
		}

		static std::filesystem::path get_executable_directory()
		{
#ifdef _WIN32
			std::vector<char> buf(MAX_PATH);
			DWORD len = GetModuleFileNameA(NULL, buf.data(), static_cast<DWORD>(buf.size()));
			while (len == static_cast<DWORD>(buf.size()) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				buf.resize(buf.size() * 2);
				len = GetModuleFileNameA(NULL, buf.data(), static_cast<DWORD>(buf.size()));
			}
			if (len > 0)
				return std::filesystem::path(std::string(buf.data(), len)).parent_path();
#elif defined(__linux__)
			std::error_code ec;
			auto p = std::filesystem::read_symlink("/proc/self/exe", ec);
			if (!ec)
				return p.parent_path();
#elif defined(__APPLE__)
			// TODO: implement using _NSGetExecutablePath
#endif
			return {};
		}

		// Search for a .sl file using the 5-step path and evaluate it.
		bool import_sl(const std::string& filename)
		{
			for (const auto& dir : make_search_path())
			{
				auto full = dir / filename;
				if (!std::filesystem::exists(full))
					continue;
				std::ifstream t(full);
				if (!t.good())
					continue;
				std::stringstream buffer;
				buffer << t.rdbuf();
				script_dirs_.push_back(std::filesystem::absolute(full).parent_path());
				try
				{
					auto ast = simpl::parse(buffer.str());
					evaluate(ast);
				}
				catch (const token_error& te)
				{
					script_dirs_.pop_back();
					throw std::runtime_error(detail::format("{0}: error: {1}", std::filesystem::absolute(full).string(), te.what()));
				}
				catch (const parse_error& pe)
				{
					script_dirs_.pop_back();
					throw std::runtime_error(detail::format("{0}: error: {1}", std::filesystem::absolute(full).string(), pe.what()));
				}
				catch (...)
				{
					script_dirs_.pop_back();
					throw;
				}
				script_dirs_.pop_back();
				return true;
			}
			return false;
		}

		// Native library entry point convention:
		//   extern "C" void simpl_load(simpl::vm* vm);
		bool import_native(const std::string& filename)
		{
			for (const auto& dir : make_search_path())
			{
				auto full = std::filesystem::absolute(dir / filename);
				if (!std::filesystem::exists(full))
					continue;
				auto full_str = full.string();
#ifdef _WIN32
				HMODULE h = LoadLibraryA(full_str.c_str());
				if (!h)
				{
					DWORD err = GetLastError();
					char msg[256] = {};
					FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, err, 0, msg, sizeof(msg), NULL);
					throw std::runtime_error(detail::format("failed to load native library '{0}': {1}", full_str, msg));
				}
				using load_fn_t = void(*)(vm*);
				auto fn = reinterpret_cast<load_fn_t>(GetProcAddress(h, "simpl_load"));
				if (!fn)
					throw std::runtime_error(detail::format("native library '{0}' does not export 'simpl_load'.", full_str));
				fn(&vm_);
#else
				void* h = dlopen(full_str.c_str(), RTLD_LAZY | RTLD_GLOBAL);
				if (!h)
					throw std::runtime_error(detail::format("failed to load native library '{0}': {1}", full_str, dlerror()));
				using load_fn_t = void(*)(vm*);
				auto fn = reinterpret_cast<load_fn_t>(dlsym(h, "simpl_load"));
				if (!fn)
					throw std::runtime_error(detail::format("native library '{0}' does not export 'simpl_load'.", full_str));
				fn(&vm_);
#endif
				return true;
			}
			return false;
		}

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
			auto s1 = vm_.stack_size();
			for (const auto &expr : exp.expressions())
				expr->evaluate(*this);			
			size_t arity = vm_.stack_size() - s1;
			detail::call_def cd{ exp.identifier().name, make_arg_list(vm, arity) };
			vm_.call(cd);
			vm_.decrement_stack(arity);
		}

		void do_expand(nary_expression &exp, vm &vm)
		{
			exp.expressions()[0]->evaluate(*this);
			// top of the stack should be an array, now we'll expand it.
			auto top = vm_.pop_stack();
			if (!std::holds_alternative<arrayref_t>(top))
				throw std::runtime_error("invalid expansion.");
			arrayref_t arr = std::get<arrayref_t>(top);
			for (const auto &v : arr->values)
			{
				vm.push_stack(v);
			}
		}

		/// TODO: Refactor to use apply<op> pattern.
		void do_increment(nary_expression& exp, vm& vm)
		{
			if (std::holds_alternative<identifier>(exp.expressions()[0]->value()))
			{
				// pre
				const auto& id = std::get<identifier>(exp.expressions()[0]->value());
				auto& value = std::get<number>(vm_.load_var(id));
				++value;
				vm_.set_val(id, value_t{ value });
				vm_.push_stack(value_t{ value });
			}
			else 
			{
				// post-increment; i=i++;
				const auto& id = std::get<identifier>(exp.expressions()[1]->value());
				auto value = std::get<number>(vm_.load_var(id));
				vm_.push_stack(value_t{ value });
				++value;
				vm_.set_val(id, value_t{ value });
			}
		}

		void do_decrement(nary_expression& exp, vm& vm)
		{
			if (std::holds_alternative<identifier>(exp.expressions()[0]->value()))
			{
				// pre
				const auto& id = std::get<identifier>(exp.expressions()[0]->value());
				auto& value = std::get<number>(vm_.load_var(id));
				--value;
				vm_.set_val(id, value_t{ value });
				vm_.push_stack(value_t{ value });
			}
			else
			{
				// post-increment; i=i++;
				const auto& id = std::get<identifier>(exp.expressions()[1]->value());
				auto value = std::get<number>(vm_.load_var(id));
				vm_.push_stack(value_t{ value });
				--value;
				vm_.set_val(id, value_t{ value });
			}
		}

		void do_assignment(nary_expression &exp, vm &vm)
		{
			exp.expressions()[0]->evaluate(*this);			
			const auto &id = std::get<identifier>(exp.expressions()[1]->value());
			vm_.set_val(id, 0);
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
			auto &val = vm_.load_var(id);
			vm_.push_stack(val);
		}

	private:
		vm &vm_;
		std::vector<std::string> importing_;
		std::vector<std::string> imported_;
		std::vector<std::filesystem::path> script_dirs_;
	};
}

#endif // __simpl_vm_execution_context_h__