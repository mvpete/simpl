#include <string>

#include "parser.h"
#include "evaluate.h"

int main()
{
	std::string prg("print(\"hello world\");");
	//std::string prg("print(4/2+2*4*6);");

	simpl::parser<std::string::iterator> parser(prg.begin(), prg.end());

	simpl::vm vm;
	evaluate(parser.next(), vm);

}