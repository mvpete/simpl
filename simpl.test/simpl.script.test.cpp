#include "pch.h"
#include "CppUnitTest.h"

#include <simpl/script.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace simpl_test
{
	class foo
	{
		simpl::number my_count_;
	public:
		foo()
		{
			static simpl::number count = 0;
			my_count_ = ++count;
		}		

		simpl::number bar()
		{
			return my_count_;
		}
	};

	TEST_CLASS(simpl_script_test)
	{
	public:

		TEST_METHOD(test_simpl_script_simple)
		{
			bool called = false;

			simpl::script s;
			s.register_function("foo", [&called](const std::string& s)
			{
				called = true;
				Assert::AreEqual(std::string{ "bar" }, s);
			});

			s.evaluate("foo(\"bar\");");

			Assert::IsTrue(called);
		}

		TEST_METHOD(test_simpl_script_reg_type)
		{
			simpl::script s;
			s.register_type<foo>();

			bool called = false;
			s.register_function("foo_bar", [&called](foo& f)
			{
				called = true;
				return f.bar();
			});

			s.evaluate("let f=make_foo(); foo_bar(f);");
			Assert::IsTrue(called);
		}

		TEST_METHOD(test_simpl_script_make)
		{
			simpl::script s;
			s.register_type<foo>();

		}
	};
}

SIMPL_TYPE_DEF(simpl_test::foo, "foo");

