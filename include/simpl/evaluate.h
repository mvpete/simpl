#ifndef __simpl_evaluate_h__
#define __simpl_evaluate_h__

#include <simpl/engine.h>

namespace simpl
{
	void evaluate(statement_ptr statement, engine &e)
	{
		if(statement)
			statement->evaluate(e.context());
	}
}

#endif //__simpl_evaluate_h__
