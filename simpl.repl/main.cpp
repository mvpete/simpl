#include <string>
#include <fstream>
#include <chrono>
#include <iostream>
#include <sstream>

#define SIMPL_DEFINES
#include <simpl/simpl.h>

struct exit_ {};

namespace
{
	void print_help()
	{
		std::cout << "\r\nSIMPL REPL commands:\r\n"
			<< "  :help      Show this help\r\n"
			<< "  :examples  Show quick SIMPL examples\r\n"
			<< "  :quit      Exit the REPL\r\n"
			<< "You can also call exit(); from SIMPL code.\r\n\r\n";
	}

	void print_examples()
	{
		std::cout << "\r\nExamples:\r\n"
			<< "  @import io\r\n"
			<< "  println(\"hello world\");\r\n"
			<< "  let i = 0;\r\n"
			<< "  i++;\r\n"
			<< "  println(i);\r\n\r\n";
	}
}

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

		std::string script_buffer;
		while (s.good())
		{
			try
			{
				std::cout << prompt;
				std::string line;
				std::getline(s, line);
				if (line.empty())
					continue;

				if (line == ":help")
				{
					print_help();
					continue;
				}
				if (line == ":examples")
				{
					print_examples();
					continue;
				}
				if (line == ":quit")
				{
					throw exit_{};
				}

				if (!script_buffer.empty())
				{
					script_buffer += "\n";
				}
				script_buffer += line;
				simpl::parser parser(script_buffer);
				
				auto stmt = parser.next();
				if (stmt == nullptr) 
				{
					prompt = "+";
					continue;
				}
				else 
				{
					evaluate(std::move(stmt), e);
					script_buffer.clear();
					prompt = ">";
					std::cout << std::endl;
				}

			}
			catch (const simpl::token_error &t)
			{
				std::cout << "\r\n";
				std::cout << "failed to tokenize input '" << script_buffer << "' - " << t.what() << std::endl;
				script_buffer.clear();
				prompt = ">";
			}
			catch (const simpl::parse_error &p)
			{
				std::cout << "\r\n";
				std::cout << "failed to parse input '" << script_buffer << "' - " << p.what() << std::endl;
				script_buffer.clear();
				prompt = ">";
			}
			catch (const std::exception &e)
			{
				std::cout << "\r\n";
				std::cout << e.what() << std::endl;
				script_buffer.clear();
				prompt = ">";
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
		std::chrono::time_point now = std::chrono::high_resolution_clock::now();
		run_string(str);
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count();
		std::cout << "\r\n\r\nelapsed: " << elapsed << " ms.";
	}
	else
	{
		return run_interpreter(std::cin);
	}
}