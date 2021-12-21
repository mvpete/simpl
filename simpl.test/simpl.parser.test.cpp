#include "pch.h"
#include "CppUnitTest.h"

#define SIMPL_DEFINES
#include <simpl/simpl.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace simpl_test
{
	TEST_CLASS(simpl_parser_test)
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
			auto ast = simpl::parse("for(let i=0; i<5; i=i+1)");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];
			auto whle = dynamic_cast<simpl::for_statement *>(stmt.get());
			Assert::IsNotNull(whle);
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
			auto ast = simpl::parse("if(1) { 1+2; } else { i; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];
			auto iff = dynamic_cast<simpl::if_statement *>(stmt.get());
			Assert::IsNotNull(iff);
		}

		TEST_METHOD(TestParseIfElseIfStatement)
		{
			auto ast = simpl::parse("if(1) { 1+2; } else if (0) { i; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];
			auto iff = dynamic_cast<simpl::if_statement *>(stmt.get());
			Assert::IsNotNull(iff);
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

		TEST_METHOD(TestParseSimplObjectEmpty)
		{
			auto ast = simpl::parse("object obj{}");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];

			auto expr = dynamic_cast<simpl::object_definition_statement *>(stmt.get());
			Assert::IsNotNull(expr);

			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::AreEqual(size_t{ 0 }, expr->members().size());

		}

		TEST_METHOD(TestParseSimpleObjectWMembers)
		{
			auto ast = simpl::parse("object obj { mem1; mem2; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];

			const auto expr = dynamic_cast<const simpl::object_definition_statement *>(stmt.get());
			Assert::IsNotNull(expr);
			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::AreEqual(size_t{ 2 }, expr->members().size());
		}

		TEST_METHOD(TestParseSimplObjectWInitializer)
		{
			auto ast = simpl::parse("object obj { mem1=1; mem2=\"king\"; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];

			const auto expr = dynamic_cast<const simpl::object_definition_statement *>(stmt.get());
			Assert::IsNotNull(expr);

			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::AreEqual(size_t{ 2 }, expr->members().size());
		}

		TEST_METHOD(TestParseSimpleObjectInherits)
		{
			auto ast = simpl::parse("object obj inherits obj2 { }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto &stmt = ast[0];

			const auto expr = dynamic_cast<const simpl::object_definition_statement *>(stmt.get());
			Assert::IsNotNull(expr);

			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::IsTrue(expr->inherits().has_value());
			Assert::AreEqual(std::string("obj2"), expr->inherits().value());
		}

	};
}
