#ifndef __simpl_evaluate_h__
#define __simpl_evaluate_h__

#include <simpl/vm.h>
#include <simpl/vm_execution_context.h>

namespace simpl
{
	void evaluate(statement_ptr statement, vm_execution_context &ctx)
	{
		if(statement)
			statement->evaluate(ctx);
	}
}

#endif //__simpl_evaluate_h__
