#ifndef __simpl_vm_h__
#define __simpl_vm_h__

#include "value.h"
#include "static_stack.h"

#include <iostream>
#include <functional>
#include <map>
#include <stack>

namespace simpl
{
    struct fn_def
    {
        std::string name;
        size_t arity;
        std::function<void()> fn;
    };

    class vm
    {
        class var_scope
        {
            std::map<std::string, value_t *> variables_;
        public:
            var_scope()
            {
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
        };

        detail::static_stack<var_scope, 4096> closures_;
        std::map<std::string, fn_def> functions_;
        detail::static_stack<value_t, 4096> stack_;
    public:

        vm()
        {
            closures_.push(var_scope{});
        }

        void call(const std::string &fn_s)
        {
            auto fn = functions_.find(fn_s);
            if (fn != functions_.end())
            {
                if (fn->second.arity > stack_.size())
                    throw std::runtime_error("arity error");
                fn->second.fn();
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

        void decrement_stack(size_t offset)
        {
            stack_.pop(offset);
        }

        value_t &stack_offset(size_t offset)
        {
            return stack_.offset(offset);
        }

        template <typename CallableT>
        void reg_fn(const std::string &id, size_t arity, CallableT &&fn)
        {
            reg_fn(fn_def{ id, arity, fn });
        }

        void reg_fn(fn_def &&df)
        {
            auto name = df.name;
            if (functions_.find(name) != functions_.end())
            {
                std::stringstream ss;
                ss << "function '" << name << "' already defined.";
                throw std::runtime_error(ss.str());
            }
            functions_[df.name] = std::move(df);
        }

        void create_var(const std::string &name, size_t offset = 0)
        {
            closures_.top().track(name, &stack_.offset(offset));
        }
        void set_val(const std::string &name, size_t offset)
        {
            auto stack = 0;
            while (stack < closures_.size())
            {
                auto scope = closures_.offset(stack);
                if (scope.has_value(name))
                {
                    return scope.set_value(name, stack_offset(offset));
                }
                ++stack;
            }

            closures_.top().set_value(name, stack_.offset(offset));
        }

        value_t &load_var(const std::string &name)
        {
            auto offset = 0;
            while (offset < closures_.size())
            {
                auto scope = closures_.offset(offset);
                if (scope.has_value(name))
                {
                    return scope.get_value(name);
                }
                ++offset;
            }
            std::stringstream ss;
            ss << "undefined variable '" << name << "'";
            throw std::runtime_error(ss.str());
        }

        void enter_scope()
        {
            closures_.push(var_scope{});
        }

        void exit_scope()
        {
            if (!closures_.empty())
                closures_.pop();
        }

    private:
        std::vector<std::reference_wrapper<simpl::value_t>> make_stack_args(size_t count)
        {
            std::vector<std::reference_wrapper<simpl::value_t>> args;
            for (size_t i = 0; i < count; ++i)
            {
                args.push_back(std::ref(stack_.offset(i)));
            }
            return args;
        }

    };



}


#endif //__simpl_vm_h__