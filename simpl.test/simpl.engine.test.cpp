#include "pch.h"
#include "CppUnitTest.h"

#include <simpl/simpl.h>

#include <array>
#include <functional>
#include <optional>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace simpl_test
{
	TEST_CLASS(simpl_engine_test)
	{
	private:
		simpl::engine e;
		std::function<void()> trap;
		std::function<void(const simpl::value_t&)> check;

	public:
		simpl_engine_test()
		{
			e.machine().reg_fn("dbg_break", [this]()
			{
				if(trap != nullptr)
					trap();
			});

			e.machine().reg_fn("assert", [this](const simpl::value_t& v)
			{
				if (check != nullptr)
					check(v);
			});
		}

		TEST_METHOD_INITIALIZE(TestInit)
		{
			trap = nullptr;
			check = nullptr;
		}
		
		TEST_METHOD(TestObjectTypeRegistration)
		{
			auto ast = simpl::parse("object test_object { mem1; mem2; }");
			simpl::evaluate(ast, e);

			Assert::IsTrue(e.machine().has_type("test_object"));
		}

		TEST_METHOD(TestFunctionCall)
		{
			auto ast = simpl::parse("def foo() { dbg_break(); } foo();");

			bool called = false;
			trap = [&, this]()
			{
				called = true;

				// main -> foo -> dbg_break
				Assert::AreEqual(size_t{ 3 }, e.machine().callstack().size());
				Assert::AreEqual(size_t{ 3 }, e.machine().scopes().size());

				// 2 retvals foo & dbg_break, no main retval
				Assert::AreEqual(size_t{ 2 }, e.machine().stack().size());

			};

			simpl::evaluate(ast, e);

			Assert::IsTrue(called);

		}

		TEST_METHOD(TestArrayExplosionSimple)
		{
			bool called = false;
			e.machine().reg_fn("kaboom", [&](const std::string& text, simpl::number number)
			{
				called = true;

				Assert::AreEqual(std::string{ "foo" }, text);
				Assert::AreEqual(42.0, number);

			});

			auto ast = simpl::parse("let arr = new [ \"foo\", 42 ]; kaboom(arr...);");

			simpl::evaluate(ast, e);

			Assert::IsTrue(called);
		}

		TEST_METHOD(TestArrayExplosionTypeMismatch)
		{
			Assert::ExpectException<std::runtime_error>([&]()
			{
				bool called = false;
				e.machine().reg_fn("kaboom", [&](const std::string& text, simpl::number number)
				{
					called = true;

					Assert::AreEqual(std::string{ "foo" }, text);
					Assert::AreEqual(42.0, number);

				});

				auto ast = simpl::parse("let arr = new [ 13, 42 ]; kaboom(arr...);");

				simpl::evaluate(ast, e);
			});
		}

		TEST_METHOD(TestNonArrayExplosion)
		{
			Assert::ExpectException<std::runtime_error>([&]()
			{
				bool called = false;
				e.machine().reg_fn("kaboom", [&](simpl::number number)
				{
					called = true;

					Assert::AreEqual(42.0, number);

				});

				auto ast = simpl::parse("let arr = 13; kaboom(arr...);");

				simpl::evaluate(ast, e);
			});
		}

		TEST_METHOD(TestFunctionAddress)
		{
			bool called = false;
			e.machine().reg_fn("kaboom", [&](const std::string &name)
			{
				called = true;

				Assert::AreEqual(std::string{ "foo" }, name);

			});

			auto ast = simpl::parse("def foo() {} kaboom(&foo);");

			simpl::evaluate(ast, e);
		}

		TEST_METHOD(TestPostIncrement)
		{
			bool called = false;
			check = [&](const simpl::value_t& v)
			{
				called = true;
				Assert::IsTrue(std::holds_alternative<simpl::number>(v));
				Assert::AreEqual(1.00, std::get<simpl::number>(v));
			};
			auto ast = simpl::parse("let i =0; i++; assert(i);");
			simpl::evaluate(ast, e);
			Assert::IsTrue(called);
		}

		TEST_METHOD(TestPostIncrementReturnValue)
		{
			bool called = false;
			check = [&](const simpl::value_t& v)
			{
				called = true;
				Assert::IsTrue(std::holds_alternative<simpl::number>(v));
				Assert::AreEqual(2.00, std::get<simpl::number>(v));
				
			};
			auto ast = simpl::parse("let j=0; let i =2; j=i++; assert(j);");
			simpl::evaluate(ast, e);
			Assert::IsTrue(called);
		}

		TEST_METHOD(TestPostDecrement)
		{
			bool called = false;
			check = [&](const simpl::value_t& v)
			{
				called = true;
				Assert::IsTrue(std::holds_alternative<simpl::number>(v));
				Assert::AreEqual(-1.00, std::get<simpl::number>(v));
			};
			auto ast = simpl::parse("let i =0; i--; assert(i);");
			simpl::evaluate(ast, e);
			Assert::IsTrue(called);
		}

		TEST_METHOD(TestPreIncrement)
		{
			bool called = false;
			check = [&](const simpl::value_t& v)
			{
				called = true;
				Assert::IsTrue(std::holds_alternative<simpl::number>(v));
				Assert::AreEqual(1.00, std::get<simpl::number>(v));
			};
			auto ast = simpl::parse("let i =0; ++i; assert(i);");
			simpl::evaluate(ast, e);
			Assert::IsTrue(called);
		}

		TEST_METHOD(TestPreDecrement)
		{
			bool called = false;
			check = [&](const simpl::value_t& v)
			{
				called = true;
				Assert::IsTrue(std::holds_alternative<simpl::number>(v));
				Assert::AreEqual(-1.00, std::get<simpl::number>(v));
			};
			auto ast = simpl::parse("let i=0; --i; assert(i);");
			simpl::evaluate(ast, e);
			Assert::IsTrue(called);
		}

		TEST_METHOD(TestSingleFunctionCallNoArgs)
		{
			bool called = false;
			trap = [&]()
			{
				called = true;
				Assert::AreEqual((size_t)1, e.machine().stack().size());
			};
			auto ast = simpl::parse("dbg_break();");
			simpl::evaluate(ast, e);
			Assert::IsTrue(called);
		}

		TEST_METHOD(TestSingleFunctionCallWithArgs)
		{
			trap = [&]()
			{
				// 1 retval, 3 args, 1 retval
				Assert::AreEqual((size_t)5, e.machine().stack().size());
			};
			auto ast = simpl::parse("def test(a, b, c) { dbg_break(); } test(1,2,3); ");
			simpl::evaluate(ast, e);
			
			trap = [&]()
			{
				// check that the stack was cleaned up.
				Assert::AreEqual((size_t)1, e.machine().stack().size());
			};
			ast = simpl::parse("dbg_break();");
			simpl::evaluate(ast, e);
		}

		TEST_METHOD(TestMethodAssignmentThenExplode) 
		{
			e.machine().reg_fn("kaboom", [](const std::string& a, simpl::number b, const std::string& c)
			{
				Assert::AreEqual(std::string{ "foo" }, a);
				Assert::AreEqual(simpl::number{ 42.00 }, b);
				Assert::AreEqual(std::string{ "bar" }, c);
			});

			e.machine().reg_fn("get_foo", []()
			{
				return std::string{ "foo" };
			});

			auto ast = simpl::parse("let line = 0; def get_item() { line = get_foo(); } get_item(); let arr = new [ line, 42, \"bar\"]; kaboom(arr...);");
			simpl::evaluate(ast, e);		
		}

		TEST_METHOD(TestSimpleLetStatement)
		{
			// Stack will be 0 +1 for the dbg_break() retval, 1 +1 for the dbg_break(); retval.
			std::array<size_t, 2> sizes = { 1, 2 };
			size_t i = 0;
			trap = [&]()
			{
				Assert::AreEqual(sizes[i++], e.machine().stack().size());
			};
			run("dbg_break(); let foo=4; dbg_break();");
		}

		TEST_METHOD(TestScopedLetStatement)
		{
			std::array<size_t, 3> sizes = { 2, 3, 1 };
			size_t i = 0;
			trap = [&]()
			{
				Assert::AreEqual(sizes[i++], e.machine().stack().size());
			};
			run("def foo() { dbg_break(); let i=128; dbg_break(); let j=256; } foo(); dbg_break();");
		}

		TEST_METHOD(TestIncrementArrayItem)
		{
			check = [](const simpl::value_t& v)
			{
				Assert::IsTrue(std::holds_alternative<simpl::number>(v));
				Assert::AreEqual((simpl::number)2.00, std::get<simpl::number>(v));
			};

			run("let arr = new [ \"item\", 1, new {}]; ++arr[1]; assert(arr[1]);");
		}

		TEST_METHOD(TestExpandCleansUp)
		{
			std::array<size_t, 3> sizes = { 2, 2 };
			size_t i = 0;
			trap = [&]()
			{
				Assert::AreEqual(sizes[i++], e.machine().stack().size());
			};
			run("let foo = new [1, 2, 3]; dbg_break(); foo...; dbg_break();");
		}

		TEST_METHOD(TestExpandMethodCallStackSize)
		{
			std::array<size_t, 3> sizes = { 2, 6, 2 };
			size_t i = 0;
			trap = [&]()
			{
				Assert::AreEqual(sizes[i++], e.machine().stack().size());
			};
			run("def bar(a,b,c) { dbg_break(); } let foo = new [1, 2, 3]; dbg_break(); bar(foo...); dbg_break();");
		}


		TEST_METHOD(TestLetExpand)
		{
			Assert::ExpectException<std::runtime_error>([&]()
			{
				run("let foo = new [1,2,3]; let j = foo...;");
			});
		}

private:
		void run(const std::string& str)
		{
			auto ast = simpl::parse(str);
			simpl::evaluate(ast, e);
		}
	};
}
