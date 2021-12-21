#ifndef __simpl_types_h__
#define __simpl_types_h__

#include <simpl/detail/format.h>
#include <simpl/expression.h>

#include <array>
#include <list>

namespace simpl
{
    namespace detail
    {
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
                :name(std::move(td.name)), native(std::move(td.native)), inherits(std::move(td.inherits)), members(std::move(td.members))
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
            const type_def *inherits;
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

                types_.emplace_back(std::move(def));
                
                //types_[next_] = std::move(def);
                ++next_;
            }

            const type_def *get_type(const std::string &name)
            {
                for (const auto &t : types_)
                {
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
                if (t2 == "any")
                    return true; // this is a hack...
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
            std::list<detail::type_def> types_;
            size_t next_ = 0;
        };


    }
}

#endif //__simpl_types_h__