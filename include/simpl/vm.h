#ifndef __simpl_vm_h__
#define __simpl_vm_h__

#include <simpl/detail/signature.h>
#include <simpl/expression.h>
#include <simpl/static_stack.h>
#include <simpl/value.h>

#include <iostream>
#include <functional>
#include <map>
#include <stack>
#include <sstream>
#include <tuple>

namespace simpl
{
    struct call_def
    {
        std::string name;
        std::vector<std::string> arguments;
    };


    namespace detail
    {
        std::string format_name(const std::string &name, const std::vector<std::string> &arguments)
        {
            std::stringstream ss;
            ss << name << "(";
            for (size_t i = 0; i < arguments.size(); ++i)
            {
                ss << arguments[i];
                if (i != arguments.size() - 1)
                    ss << ",";
            }
            ss << ")";
            return ss.str();
        }

        struct fn_def
        {
            std::string id;
            std::string name;
            std::vector<std::string> args;
            std::function<void()> fn;
        };

        struct type_def
        {
            type_def()
                :inherits(nullptr)
            {
            }

            type_def(const std::string &name, const std::string &native)
                :name(name), native(native), inherits(nullptr)
            {
            }

            type_def(const std::string &name)
                :name(name), inherits(nullptr)
            {
            }

            type_def(const std::string &name, const type_def *inherits, std::vector<object_definition::member> &&members)
                :name(name), inherits(inherits), members(std::move(members))
            {
            }

            type_def(type_def &&td) noexcept
                :name(std::move(td.name)), inherits(std::move(td.inherits)), members(std::move(td.members))
            {
            }

            type_def &operator=(type_def &&td) noexcept
            {
                std::swap(td.name, name);
                std::swap(td.native, native);
                std::swap(td.inherits, inherits);
                std::swap(td.members, members);
                return *this;
            }

            std::string name;
            std::optional<std::string> native;
            const type_def* inherits;
            std::vector<simpl::object_definition::member> members;
        };

        class type_table
        {

        public:

            void register_type(const std::string &name, const std::optional<std::string> &inherits, std::vector<object_definition::member> &&members)
            {
                const type_def *super = nullptr;
                if (inherits.has_value())
                {
                    super = get_type(inherits.value());
                    if (super == nullptr)
                        throw std::runtime_error("type does not exist");
                }
               
                register_type(type_def{ name,super,std::move(members) });
            }

            void register_type(type_def &&def)
            {
                auto t = get_type(def.name);
                if (t != nullptr)
                    throw std::runtime_error("type exists");

                types_[next_] = std::move(def);
                ++next_;
            }

            const type_def *get_type(const std::string &name)
            {
                for (size_t i = 0; i < next_; ++i)
                {
                    const auto &t = types_[i];
                    if (t.name == name)
                        return &t;
                }
                return nullptr;
            }

            // checks if t1, is in the lineage of t2
            // bike is-a vehicle -> true
            // car  is-a vehicle -> true
            // bike is-a car     -> false
            bool is_a(const std::string &t1, const std::string &t2)
            {
                // first, find t1
                // then walk its lineage until you find t2
                const type_def *t1_p = nullptr;
                for (const auto &td : types_)
                {
                    if (td.name == t1)
                        t1_p = &td;
                }
                if (t1_p == nullptr)
                    throw std::runtime_error(detail::format("unrecognized type '{0}'", t1));

                while (t1_p != nullptr)
                {
                    if (t1_p->name == t2)
                        return true;
                    t1_p = t1_p->inherits;
                }
                return false;

            }

            std::string translate_type(const std::string &nt)
            {
                auto i = std::find_if(types_.begin(), types_.end(), [&](const auto &td)
                {
                    return td.native == nt;
                });
                if (i == types_.end())
                    throw std::runtime_error(detail::format("type '{0}' not registered", nt));
                return i->name;
            }

            std::vector<std::string> translate_types(const std::vector<std::string> &native_types)
            {
                std::vector<std::string> simpl_types;
                for (const std::string &nt : native_types)
                {
                    simpl_types.push_back(translate_type(nt));
                }
                return simpl_types;
            }

        private:
            std::array<type_def,256> types_;
            size_t next_=0;
        };

        class dispatch_table
        {
        public:
            dispatch_table(type_table &types)
                :types_(types)
            {
            }

            const fn_def *lookup(const call_def &cd)
            {
                // 1. argument specific lookup.
                // 2. backoff generic lookup.
                std::vector<std::string> args = cd.arguments;
                const fn_def *match = find_exact_match(cd.name, args);

                if (match != nullptr)
                    return match;

                // find all candidate functions w/ first arg in-tree
                auto candidates = find_candidate_functions(cd.name, args);
                
                if (candidates.size() > 1)
                    throw std::runtime_error("multiple matches for function"); // remember this word...

                if (candidates.size() == 0)
                    throw std::runtime_error("no function match");               

                return candidates[0];
            }
            void register_function(fn_def &&df)
            {
                auto name = df.id;
                if (functions_.find(name) != functions_.end())
                {
                    throw std::runtime_error(detail::format("function '{0}' already defined", name));
                }
                functions_[name] = std::move(df);

            }

        private:

            std::vector<const fn_def *> find_candidate_functions(const std::string &name, const std::vector<std::string> &args_t)
            {
                // build the inheritance tree
                std::vector<const fn_def *> candidates;
                for (const auto &fn : functions_)
                {
                    const auto &fn_d = fn.second;
                    const auto &args = fn.second.args;
                    if(fn_d.name == name && fn_d.args.size() == args_t.size() && (args_t.size() == 0 || types_.is_a(args_t[0],args[0]) || args[0] == "any"))
                        candidates.push_back(&fn.second);
                }

                candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                [&](const fn_def *fn) 
                { 
                    for (size_t i = 1; i < args_t.size(); ++i)
                    {
                        if (!types_.is_a(args_t[i], fn->args[i]) || fn->args[i] == "any")
                            return true;
                    }
                    return false;
                }), candidates.end());
                

                return candidates;
            }

            const fn_def *find_exact_match(const std::string &name, const std::vector<std::string> &args)
            {
                auto call_id = detail::format_name(name, args);
                auto match = functions_.find(call_id);
                if (match == functions_.end())
                    return nullptr;
                return &match->second;
            }

        private:
            type_table &types_;
            std::map<std::string, fn_def> functions_;
        };
    }

    class vm
    {
        class var_scope
        {
        public:
            var_scope() :vm_(nullptr) {}
            var_scope(vm &vm)
                :vm_(&vm)
            {
            
            }
            var_scope(const var_scope &) = delete;
            var_scope(var_scope &&rhs) noexcept
                :vm_(nullptr)
            {
                std::swap(vm_, rhs.vm_);
                std::swap(variables_, rhs.variables_);
            }
            var_scope &operator=(const var_scope &) = delete;
            var_scope &operator=(var_scope &&rhs) noexcept
            {
                std::swap(vm_, rhs.vm_);
                std::swap(variables_, rhs.variables_);
                return *this;
            }
            ~var_scope()
            {
                if (vm_ == nullptr)
                    return;
                //for (size_t i = 0; i < variables_.size(); ++i)
                //    vm_->pop_stack();

            }
            void track(const std::string &name, value_t *v)
            {
                if (variables_.find(name) != variables_.end())
                    throw std::runtime_error("already defined");
                variables_[name] = v;
            }
            void set_value(const std::string &name, value_t &value)
            {
                if (variables_.find(name) == variables_.end())
                    throw std::runtime_error("undefined variable");
                (*variables_[name]) = value;
            }
            value_t &get_value(const std::string &name)
            {
                if (variables_.find(name) == variables_.end())
                {
                    std::stringstream ss;
                    ss << "undefined variable '" << name << "'";
                    throw std::runtime_error(ss.str());
                }
                return *variables_[name];
            }
            bool has_value(const std::string &name)
            {
                return variables_.find(name) != variables_.end();
            }

        private:
            vm *vm_;
            std::map<std::string, value_t *> variables_;
        };

        struct activation_record
        {
            activation_record() :retval(nullptr) {};
            activation_record(const std::string &name, value_t *retval)
                :function(name), retval(retval)
            {
            }
            std::string function;
            value_t *retval;
        };   

        template <size_t N, typename ...Args>
        struct arg_list
        {
            static std::tuple<> next(vm &vm)
            {
                return std::tuple<>{};
            }
        };

        template<size_t N, typename T, typename ...Args>
        struct arg_list<N, T, Args...>
        {
            static std::tuple<std::reference_wrapper<T>, std::reference_wrapper<Args>...> next(vm &vm)
            {
                return std::tuple_cat(std::make_tuple(std::ref(detail::get_value<T>(vm.stack_offset(N - 1)))), arg_list<N - 1, Args...>::next(vm));
            }
        };

        template <typename T>
        struct deducer {};

        template <typename ...Args>
        std::tuple<std::reference_wrapper<Args>...> load_args(vm &vm, deducer<std::tuple<Args...>>)
        {
            auto args = arg_list<sizeof...(Args), Args...>::next(vm);
            return args;
        }

    public:
        static constexpr size_t Stack_Size = 128;
        using callstack_t = detail::static_stack<activation_record, Stack_Size>;

    public:

        vm()
            :functions_(types_)
        {
            locals_.push(var_scope{*this}); // global scope.
            callstack_.push(activation_record{}); // main..

            // builtins?
            register_type<value_t>("any");
            register_type<std::string>("string");
            register_type<bool>("bool");
            register_type<double>("number");
            register_type<blob_t>("blob");
            register_type<array_t>("array");

        }

        void call(const call_def &cd)
        {
            auto fn = functions_.lookup(cd);
            activate_function(fn->name, fn->args.size());
            auto sz = callstack_.size();
            fn->fn();
            // if we run the function, and there's no return, the activation record will still exist
            if (callstack_.size() == sz)
            {
                stack_.push(value_t{}); // undefined.
                return_(); // implicit return...
            }
        }

        void push_stack(const value_t &v)
        {
            stack_.push(v);
        }

        value_t pop_stack()
        {
            if (stack_.empty())
                throw std::runtime_error("stack is empty");

            const auto top = stack_.top();
            stack_.pop();
            return top;
        }

        size_t stack_size()
        {
            return stack_.size();
        }

        void decrement_stack(size_t offset)
        {
            stack_.pop(offset);
        }

        value_t &stack_offset(size_t offset)
        {
            return stack_.offset(offset);
        }
      
        size_t depth() const
        {
            return callstack_.size();
        }

        void activate_function(const std::string &fn, size_t retval_offset)
        {
            enter_scope();
            callstack_.push(activation_record{ fn, &stack_.offset(retval_offset) });
        }

        void return_()
        {
            if (callstack_.size() == 1)
                throw std::runtime_error("bad return statement");
            auto &ar = callstack_.top();
            *ar.retval = stack_.top();
            stack_.pop();
            callstack_.pop();
            exit_scope();
        }

        template <typename CallableT>
        void reg_fn(const std::string &name, CallableT &&fn)
        {
            constexpr auto sig = detail::get_signature<CallableT>();
            const auto id = detail::format("{0}({1})", name, sig.arguments_string(types_));
            const auto args = types_.translate_types(sig.arguments());
            reg_fn(id, name, args, [this, sig, fn]()
            {
                auto args = load_args(*this, deducer<typename detail::signature<CallableT>::types>{});
                if constexpr(std::is_same_v<detail::signature<CallableT>::result_type, void>)
                {
                    std::apply(fn, args);
                    push_stack(empty_t{});
                }
                else 
                {
                    push_stack(std::apply(fn, args));
                }
            });
        }

        void reg_fn(detail::fn_def &&df)
        {
            functions_.register_function(std::move(df));
        }

        template<typename T>
        void register_type(const std::string &simpl_name)
        {
            if (has_type(simpl_name))
                throw std::runtime_error(detail::format("type '{0}' already registered", simpl_name));

            types_.register_type(detail::type_def{ simpl_name, typeid(T).name() });
        }

        void register_type(const std::string &type, const std::optional<std::string> &inherits, std::vector<object_definition::member> &&members)
        {
            if (has_type(type))
                throw std::runtime_error(detail::format("type '{0}' already registered", type));

            const detail::type_def *parent_type=nullptr;
            if (inherits.has_value())
            {
                const auto parent = lookup_type(inherits.value());
                if(parent == nullptr)
                    throw std::runtime_error(detail::format("cannot inherit '{0}' type does not exist.", inherits.value()));
                parent_type = parent;
            }

            types_.register_type(detail::type_def{ type, parent_type, std::move(members) });
        }

        bool has_type(const std::string &simpl_type)
        {
            auto td = types_.get_type(simpl_type);
            return td != nullptr;
        }

        const detail::type_def* lookup_type(const std::string &simpl_name)
        {
            return types_.get_type(simpl_name);                 
        }

        void create_var(const std::string &name, size_t offset = 0)
        {
            locals_.top().track(name, &stack_.offset(offset));
        }

        void set_val(const std::string &name, size_t offset)
        {
            size_t stack = 0;
            while (stack < locals_.size())
            {
                auto &scope = locals_.offset(stack);
                if (scope.has_value(name))
                {
                    return scope.set_value(name, stack_offset(offset));
                }
                ++stack;
            }

            locals_.top().set_value(name, stack_.offset(offset));
        }

        value_t &load_var(const std::string &name)
        {
            size_t offset = 0;
            while (offset < locals_.size())
            {
                auto &scope = locals_.offset(offset);
                if (scope.has_value(name))
                {
                    return scope.get_value(name);
                }
                ++offset;
            }
            throw std::runtime_error(detail::format("undefined variable '{0}'", name));
        }

        bool in_scope(const std::string &name)
        {
            size_t offset = 0;
            while (offset < locals_.size())
            {
                auto &scope = locals_.offset(offset);
                if (scope.has_value(name))
                {
                    return true;
                }
                ++offset;
            }
            return false;
        }

        void enter_scope()
        {
            locals_.push(var_scope{*this});
        }

        void exit_scope()
        {
            if (!locals_.empty())
                locals_.pop();
        }



    private:
        template <typename CallableT>
        void reg_fn(const std::string &id, const std::string &name, const std::vector<std::string> &args, CallableT &&fn)
        {
            reg_fn(detail::fn_def
            {
                id,
                name,
                args,
                [this,fn = std::move(fn)]()
                {
                    fn();
                    return_();
                }
            });
        }

    private:

        detail::type_table types_;
        detail::dispatch_table functions_;
        detail::static_stack<value_t, Stack_Size> stack_;
        detail::static_stack<var_scope, Stack_Size> locals_;
        callstack_t callstack_;

    };



}


#endif //__simpl_vm_h__