#ifndef __simpl_evaluate_h__
#define __simpl_evaluate_h__

#include "vm.h"
#include "statement.h"

namespace simpl
{
	void evaluate(statement_ptr statement, vm &vm)
	{
		statement->evaluate(vm);
	}
}

#endif //__simpl_evaluate_h__
