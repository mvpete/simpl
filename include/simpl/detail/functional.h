#ifndef __simpl_functional_h__
#define __simpl_functional_h__

#include <simpl/detail/format.h>
#include <simpl/detail/types.h>

#include <functional>
#include <sstream>

namespace simpl
{
    namespace detail
    {
        inline std::string format_name(const std::string &name, const std::vector<std::string> &arguments)
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

        struct call_def
        {
            std::string name;
            std::vector<std::string> arguments;
        };

        struct fn_def
        {
            std::string id;
            std::string name;
            std::vector<std::string> args;
            std::function<void()> fn;
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
                    throw std::runtime_error(detail::format("ambiguous function call: '{0}'", cd.name));

                if (candidates.size() == 0)
                    throw std::runtime_error(detail::format("no matching function found: '{0}'", cd.name));

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
                    if (fn_d.name == name && fn_d.args.size() == args_t.size() && (args_t.size() == 0 || types_.is_a(args_t[0], args[0])))
                        candidates.push_back(&fn.second);
                }

                candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
                    [&](const fn_def *fn)
                {
                    for (size_t i = 1; i < args_t.size(); ++i)
                    {
                        if (!types_.is_a(args_t[i], fn->args[i]))
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
}

#endif // __simpl_functional_h__