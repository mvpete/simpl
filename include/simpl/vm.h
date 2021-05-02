#ifndef __simpl_vm_h__
#define __simpl_vm_h__

#include <simpl/detail/signature.h>
#include <simpl/value.h>
#include <simpl/static_stack.h>

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
        struct fn_def
        {
            std::string name;
            size_t arity;
            std::function<void()> fn;
        };
        class dispatch_table
        {
        public:
            const fn_def* lookup(const call_def &cd)
            {
                // 1. argument specific lookup.
                // 2. backoff generic lookup.
                std::vector<std::string> args = cd.arguments;
                const fn_def *match = find_match(cd.name, args);
                
                if (match == nullptr)
                {
                    for (size_t i = args.size(); i > 0; --i)
                    {
                        // set the argument to generic
                        args[i-1] = detail::to_string<value_t>::value();
                        match = find_match(cd.name, args);
                        if (match != nullptr)
                            break;
                    }
                    if(match==nullptr)
                        throw std::runtime_error(detail::format("function '{0}' could not be matched", cd.name));
                }

                return match;
            }

            void register_function(fn_def &&df)
            {
                auto name = df.name;
                if (functions_.find(name) != functions_.end())
                {
                    throw std::runtime_error(detail::format("function '{0}' already defined", name));
                }
                functions_[df.name] = std::move(df);
                
            }

        private:

            const fn_def *find_match(const std::string &name, const std::vector<std::string> &args)
            {
                std::stringstream ss;
                ss << name << "(";
                for (const auto &a : args)
                    ss << a;
                ss << ")";
                auto match = functions_.find(ss.str());
                if (match == functions_.end())
                    return nullptr;
                return &match->second;
            }

        private:
            std::map<std::string, fn_def> functions_;
        };

        template <typename T>
        T get(const value_t &v)
        {
            return std::get<T>(v);
        }

        template<>
        value_t get<value_t>(const value_t &v)
        {
            return v;
        }

    }

    class vm
    {
        class var_scope
        {
            std::map<std::string, value_t *> variables_;
        public:
            var_scope() = default;
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
            static std::tuple<T, Args...> next(vm &vm)
            {
                return std::tuple_cat(std::make_tuple(detail::get<T>(vm.stack_offset(N-1))), arg_list<N-1, Args...>::next(vm));
            }
        };


        template <typename ...Args>
        std::tuple<Args...> load_args(vm &vm, std::tuple<Args...>)
        {
            return arg_list<sizeof...(Args), Args...>::next(vm);
        }
 

    public:

        using callstack_t = detail::static_stack<activation_record, 4096>;

    private:

        detail::dispatch_table functions_;
        detail::static_stack<value_t, 4096> stack_;
        detail::static_stack<var_scope, 4096> locals_;
        callstack_t callstack_;
    public:

        vm()
        {
            locals_.push(var_scope{}); // global scope.
            callstack_.push(activation_record{}); // main..
        }

        void call(const call_def &cd)
        {
            auto fn = functions_.lookup(cd);
            activate_function(fn->name, fn->arity);
            fn->fn();
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
            callstack_.push(activation_record{ fn, &stack_.offset(retval_offset) });
        }

        void return_()
        {
            if (callstack_.size() == 1)
                throw std::runtime_error("bad return statement");
            auto &ar = callstack_.top();
            *ar.retval = stack_.top();
            callstack_.pop();
        }

        template <typename CallableT>
        void reg_fn(const std::string &id, CallableT &&fn)
        {
            constexpr auto sig = detail::get_signature<CallableT>();
            const auto name = detail::format("{0}({1})", id, sig.arguments_string());
            reg_fn(name, sig.arity, [this, sig, fn]()
            {
                auto args = load_args(*this, typename detail::signature<CallableT>::types{});
                std::apply(fn, args);
            });
        }

        void reg_fn(detail::fn_def &&df)
        {
            functions_.register_function(std::move(df));
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
                auto scope = locals_.offset(stack);
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
                auto scope = locals_.offset(offset);
                if (scope.has_value(name))
                {
                    return scope.get_value(name);
                }
                ++offset;
            }
            throw std::runtime_error(detail::format("undefined variable '{0}'", name));
        }

        void enter_scope()
        {
            locals_.push(var_scope{});
        }

        void exit_scope()
        {
            if (!locals_.empty())
                locals_.pop();
        }

    private:
        template <typename CallableT>
        void reg_fn(const std::string &id, size_t arity, CallableT &&fn)
        {
            reg_fn(detail::fn_def
            {
                id,
                arity,
                [this,fn = std::move(fn)]()
                {
                    fn();
                    return_();
                }
            });
        }


    };



}


#endif //__simpl_vm_h__