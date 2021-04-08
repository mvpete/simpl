#ifndef __simpl_operations_h__
#define __simpl_operations_h__

#include <stdexcept>

#include <simpl/cast.h>
#include <simpl/value.h>

namespace simpl
{
    class invalid_operation : public std::runtime_error
    {
    public:
        invalid_operation()
            :std::runtime_error("invalid operation")
        {
        }
    };

    template <typename OpT>
    value_t apply(value_t lvalue, value_t rvalue)
    {
        OpT op(rvalue);
        std::visit(op, lvalue);
        return op.result;
    }

    template <typename ResultT>
    struct op_base
    {
        op_base(value_t &rvalue)
            :rvalue(rvalue)
        {
        }

        value_t &rvalue;
        ResultT result;
    };

    struct add_op : op_base<value_t>
    {
        add_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = lv;
        }

        void operator()(int lv)
        {
            int rv = cast<int>(rvalue);
            result = lv + rv;
        }

        void operator()(const std::string &lv)
        {
            std::string rv = cast<std::string>(rvalue);
            result = lv + rv;
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
        
    };

    struct sub_op : op_base<value_t>
    {
        sub_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = lv;
        }

        void operator()(int lv)
        {
            int rv = cast<int>(rvalue);
            result = lv - rv;
        }

        void operator()(const std::string &lv)
        {
            throw invalid_operation();
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct mult_op : op_base<value_t>
    {
        mult_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = lv;
        }

        void operator()(int lv)
        {
            int rv = cast<int>(rvalue);
            result = lv * rv;
        }

        void operator()(const std::string &lv)
        {
            throw invalid_operation();
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct div_op : op_base<value_t>
    {
        div_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = lv;
        }

        void operator()(int lv)
        {
            int rv = cast<int>(rvalue);
            result = lv / rv;
        }

        void operator()(const std::string &lv)
        {
            throw invalid_operation();
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct eqeq_op : op_base<bool>
    {
        eqeq_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = std::holds_alternative<empty_t>(rvalue);
        }

        void operator()(int lv)
        {
            result = lv == cast<int>(rvalue);
        }

        void operator()(const std::string &lv)
        {
            result = lv == cast<std::string>(rvalue);
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct neq_op : op_base<bool>
    {
        neq_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = !std::holds_alternative<empty_t>(rvalue);
        }

        void operator()(int lv)
        {
            result = lv != cast<int>(rvalue);
        }

        void operator()(const std::string &lv)
        {
            result = lv != cast<std::string>(rvalue);
        }
        
        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct lt_op : op_base<bool>
    {
        lt_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = false;
        }

        void operator()(int lv)
        {
            result = lv < cast<int>(rvalue);
        }

        void operator()(const std::string &lv)
        {
            result = lv < cast<std::string>(rvalue);
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct lte_op : op_base<bool>
    {
        lte_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = false;
        }

        void operator()(int lv)
        {
            result = lv <= cast<int>(rvalue);
        }

        void operator()(const std::string &lv)
        {
            result = lv <= cast<std::string>(rvalue);
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct gt_op : op_base<bool>
    {
        gt_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = false;
        }

        void operator()(int lv)
        {
            result = lv > cast<int>(rvalue);
        }

        void operator()(const std::string &lv)
        {
            result = lv > cast<std::string>(rvalue);
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }
    };

    struct gte_op : op_base<bool>
    {
        gte_op(value_t &rvalue)
            :op_base(rvalue)
        {
        }

        void operator()(const empty_t &lv)
        {
            result = false;
        }

        void operator()(int lv)
        {
            result = lv >= cast<int>(rvalue);
        }

        void operator()(const std::string &lv)
        {
            result = lv >= cast<std::string>(rvalue);
        }

        void operator()(const blobref_t &v)
        {
            throw invalid_operation();
        }

        void operator()(const arrayref_t &v)
        {
            throw invalid_operation();
        }


    };


}



#endif //__simpl_operations_h__