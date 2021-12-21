#include "pch.h"
#include "CppUnitTest.h"

#include <simpl/simpl.h>

#include <functional>
#include <optional>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace simpl_test
{
	TEST_CLASS(simpl_vm_test)
	{
	private:
		simpl::engine e;
		std::optional<std::function<void()>> trap;

	public:
		simpl_vm_test()
		{
			e.machine().reg_fn("dbg_break", [this]()
			{
				if(trap.has_value())
					trap.value()();
			});
		}

		TEST_METHOD_INITIALIZE(TestInit)
		{
			trap.reset();
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

	};
}
