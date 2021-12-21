#ifndef __simpl_evaluate_h__
#define __simpl_evaluate_h__

#include <simpl/engine.h>

namespace simpl
{
	inline void evaluate(statement_ptr statement, engine &e)
	{
		if(statement)
			statement->evaluate(e.context());
	}

	inline void evaluate(syntax_tree &ast, engine &e)
	{
		for (auto &stmt : ast)
		{
			evaluate(std::move(stmt), e);
		}
	}
}

#endif //__simpl_evaluate_h__
