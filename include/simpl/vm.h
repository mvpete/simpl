#ifndef __simpl_vm_h__
#define __simpl_vm_h__

#include <simpl/detail/functional.h>
#include <simpl/detail/signature.h>
#include <simpl/detail/types.h>

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
        using stack_t = detail::static_stack<value_t, Stack_Size>;
        using locals_t = detail::static_stack<var_scope, Stack_Size>;


    public:

        vm()
            :functions_(types_)
        {
            locals_.push(var_scope{*this}); // global scope.
            callstack_.push(activation_record{}); // main..
        }

        void call(const detail::call_def &cd)
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

        template<typename ...Args>
        void invoke(const std::string &method, Args &&...args)
        {
            detail::call_def cd;
            cd.name = method;
            detail::to_vector<Args...>::types(types_, cd.arguments);

            stack_.push(value_t{}); // retvall
            detail::unpack_values([this](auto t)
            {
                push_stack(value_t{ t });
            }, args...);

            call(cd);
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

        template<typename T>
        void register_type()
        {
            const auto simpl_name = detail::simple_type_info<T>::name();

            if (has_type(simpl_name))
                throw std::runtime_error(detail::format("type '{0}' already registered", simpl_name));


            const auto inherits = detail::simple_type_info<T>::inherits();
            if (inherits != nullptr)
            {
                const auto parent = types_.get_type(inherits);
                if (parent != nullptr)
                {
                    types_.register_type(detail::type_def{ simpl_name, typeid(T).name(), parent });
                    return;
                }
            }

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

    public:
        const callstack_t &callstack() const
        {
            return callstack_;
        }

        const stack_t &stack() const
        {
            return stack_;
        }

        const locals_t &scopes() const
        {
            return locals_;
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
        stack_t stack_;
        locals_t locals_;
        callstack_t callstack_;

    };
}

#endif //__simpl_vm_h__