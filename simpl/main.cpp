#include <string>
#include <fstream>

#include "parser.h"
#include "evaluate.h"

struct exit_ {};

template <typename InStream>
int run_interpreter(InStream &s, bool user)
{
	try
	{
		simpl::vm vm;
		vm.reg_fn("exit", [](simpl::vm &vm)
		{
			throw exit_{};
		});
		while (s.good())
		{
			try
			{
				if(user)
					std::cout << ">";
				std::string line;
				std::getline(s, line);
				simpl::parser parser(line.c_str(), line.c_str()+line.length());
				evaluate(parser.next(), vm);
				if(user)
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
	}
	return 0;
}

int run_string(const std::string &s)
{
	try
	{
		simpl::vm vm;
		simpl::parser parser(s.c_str(), s.c_str() + s.length());
		while (1)
		{
			auto nxt = parser.next();
			if (!nxt) break;
			evaluate(std::move(nxt), vm);
		}
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
	return 0;
}

int main(int argc, const char **argv)
{
	if (argc > 1)
	{
		const std::string file(argv[1]);
		auto fs = std::ifstream(file);
		if (!fs.good())
		{
			std::cout << "cannot open file" << std::endl;
			return -1;
		}
		std::string str((std::istreambuf_iterator<char>(fs)),std::istreambuf_iterator<char>());
		run_string(str);
	}
	else
	{
		return run_interpreter(std::cin, false);
	}
}