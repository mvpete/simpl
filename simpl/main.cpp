#include <string>

#include "parser.h"
#include "evaluate.h"

struct exit_ {};

int main()
{
	std::string prg("print(\"hello world\");");
	//std::string prg("print(4/2+2*4*6);");

	try
	{
		simpl::vm vm;
		vm.reg_fn("exit", [](simpl::vm &vm)
		{
			throw exit_{};
		});
		while (true)
		{
			try
			{
				std::cout << ">";
				std::string line;
				std::getline(std::cin, line);
				simpl::parser<std::string::iterator> parser(line.begin(), line.end());
				evaluate(parser.next(), vm);
				std::cout << std::endl;
			}
			catch (const simpl::token_error &t)
			{
				std::cout << "failed to parse - " << t.what() << std::endl;
			}
			catch (const simpl::parse_error &p)
			{
				std::cout << "failed to parse - " << p.what() << std::endl;
			}
			catch (const std::exception &e)
			{
				std::cout << e.what() << std::endl;
			}
		}

	}
	catch (const exit_ &)
	{
		return 0;
	}
}