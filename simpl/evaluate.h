#ifndef __simpl_evaluate_h__
#define __simpl_evaluate_h__

#include "vm.h"
#include "vm_execution_context.h"

namespace simpl
{
	void evaluate(statement_ptr statement, vm &vm)
	{
		vm_execution_context ctx(vm);
		if(statement)
			statement->evaluate(ctx);
	}
}

#endif //__simpl_evaluate_h__
