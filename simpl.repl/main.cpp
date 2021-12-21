#include <string>
#include <fstream>

#include <simpl/simpl.h>

struct exit_ {};

template <typename InStream>
int run_interpreter(InStream &s)
{
	std::string prompt = ">";
	try
	{
		simpl::engine e;
		e.machine().reg_fn("exit", []()
		{
			throw exit_{};
		});

		std::stringstream ss;
		while (s.good())
		{
			try
			{
				std::cout << prompt;
				std::string line;
				std::getline(s, line);
				if (line.empty())
					continue;

				ss << line;
				line = ss.str();
				simpl::parser parser(line);
				
				auto stmt = parser.next();
				if (stmt == nullptr) 
				{
					prompt = "+";
					continue;
				}
				else 
				{
					evaluate(std::move(stmt), e);
					ss.str("");
					prompt = ">";
					std::cout << std::endl;
				}

			}
			catch (const simpl::token_error &t)
			{
				std::cout << "\r\n";
				std::cout << "failed to parse - " << t.what() << std::endl;
			}
			catch (const simpl::parse_error &p)
			{
				std::cout << "\r\n";
				std::cout << "failed to parse - " << p.what() << std::endl;
			}
			catch (const std::exception &e)
			{
				std::cout << "\r\n";
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
		simpl::engine e;
		simpl::parser parser(s.c_str(), s.c_str() + s.length());
		while (1)
		{
			auto nxt = parser.next();
			if (!nxt) break;

			evaluate(std::move(nxt), e);
		}
	}
	catch (const simpl::token_error &t)
	{
		std::cout << "\r\n";
		std::cout << "failed to parse - " << t.what() << std::endl;
	}
	catch (const simpl::parse_error &p)
	{
		std::cout << "\r\n";
		std::cout << "failed to parse - " << p.what() << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << "\r\n";
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
		std::string str((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
		run_string(str);
	}
	else
	{
		return run_interpreter(std::cin);
	}
}