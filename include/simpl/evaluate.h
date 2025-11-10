#ifndef __simpl_evaluate_h__
#define __simpl_evaluate_h__

#include <simpl/statement.h>

namespace simpl
{
	template <typename EngineT>
	inline void evaluate(statement_ptr statement, EngineT &e)
	{
		if(statement)
			statement->evaluate(e.context());
	}

	template <typename EngineT>
	inline void evaluate(syntax_tree &ast, EngineT &e)
	{
		for (auto &stmt : ast)
		{
			evaluate(std::move(stmt), e);
		}
	}

}

#endif //__simpl_evaluate_h__
