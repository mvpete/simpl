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


    public:

        using callstack_t = detail::static_stack<activation_record, 4096>;

    private:


        std::map<std::string, fn_def> functions_;
        detail::static_stack<value_t, 4096> stack_;
        detail::static_stack<var_scope, 4096> locals_;
        callstack_t callstack_;
    public:

        vm()
        {
            locals_.push(var_scope{}); // global scope.
            callstack_.push(activation_record{}); // main..
        }

        void call(const std::string &fn_s)
        {
            auto fn = functions_.find(fn_s);
            if (fn != functions_.end())
            {
                if (fn->second.arity > stack_.size())
                    throw std::runtime_error("arity error");
                activate_function(fn_s, fn->second.arity);
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
        void reg_fn(const std::string &id, size_t arity, CallableT &&fn)
        {
            reg_fn(fn_def
            { 
                id, 
                arity, 
                [this,fn=std::move(fn)]()
                {
                    fn();
                    return_();
                } 
            });
        }

        template <typename CallableT>
        void reg_fn(const std::string &id, CallableT &&fn)
        {
            reg_fn(id, detail::get_signature<CallableT>().arity, []()
            {
                // unpack the signature here.

            });
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
            std::stringstream ss;
            ss << "undefined variable '" << name << "'";
            throw std::runtime_error(ss.str());
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

    };



}


#endif //__simpl_vm_h__