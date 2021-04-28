#include "pch.h"
#include "CppUnitTest.h"

#include <simpl/simpl.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace simpltest
{
	TEST_CLASS(simpltest)
	{
	public:
		
		TEST_METHOD(TestParseLetStatement)
		{
			auto ast = simpl::parse("let hw = \"hello world\";");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];

			auto let = dynamic_cast<simpl::let_statement *>(stmt.get());
			Assert::IsNotNull(let);
		}

		TEST_METHOD(TestParseWhileLoopStatement)
		{
			auto ast = simpl::parse("while(1)");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];
			auto whle = dynamic_cast<simpl::while_statement *>(stmt.get());
			Assert::IsNotNull(whle);
		}

		TEST_METHOD(TestParseForLoopStatement)
		{
			Assert::Fail();
		}

		TEST_METHOD(TestParseIfStatement)
		{
			auto ast = simpl::parse("if(1) { 1+2; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];
			auto iff = dynamic_cast<simpl::if_statement *>(stmt.get());
			Assert::IsNotNull(iff);
		}

		TEST_METHOD(TestParseIfElseStatement)
		{
			Assert::Fail();
		}

		TEST_METHOD(TestParseIfElseIfStatement)
		{
			Assert::Fail();
		}

		TEST_METHOD(TestParseComment)
		{
			auto ast = simpl::parse("# I'm a comment");
			Assert::AreEqual(size_t{ 0 }, ast.size());
		}

		TEST_METHOD(TestParseSimpleAddMathExpression)
		{
			auto ast = simpl::parse("1+2;");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];

			auto expr = dynamic_cast<simpl::expr_statement *>(stmt.get());
			Assert::IsNotNull(expr);
		}


	};
}
