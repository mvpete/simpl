#include "pch.h"
#include "CppUnitTest.h"

#include <simpl/simpl.h>

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
	};
}
