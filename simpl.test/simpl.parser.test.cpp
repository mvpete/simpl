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
			const auto& stmt = ast[0];

			auto let = dynamic_cast<simpl::let_statement*>(stmt.get());
			Assert::IsNotNull(let);
		}

		TEST_METHOD(TestParseWhileLoopStatement)
		{
			auto ast = simpl::parse("while(1)");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];
			auto whle = dynamic_cast<simpl::while_statement*>(stmt.get());
			Assert::IsNotNull(whle);
		}

		TEST_METHOD(TestParseForLoopStatement)
		{
			auto ast = simpl::parse("for(let i=0; i<5; i=i+1)");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];
			auto whle = dynamic_cast<simpl::for_statement*>(stmt.get());
			Assert::IsNotNull(whle);
		}

		TEST_METHOD(TestParseIfStatement)
		{
			auto ast = simpl::parse("if(1) { 1+2; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];
			auto iff = dynamic_cast<simpl::if_statement*>(stmt.get());
			Assert::IsNotNull(iff);
		}

		TEST_METHOD(TestParseIfElseStatement)
		{
			auto ast = simpl::parse("if(1) { 1+2; } else { i; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];
			auto iff = dynamic_cast<simpl::if_statement*>(stmt.get());
			Assert::IsNotNull(iff);
		}

		TEST_METHOD(TestParseIfElseIfStatement)
		{
			auto ast = simpl::parse("if(1) { 1+2; } else if (0) { i; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];
			auto iff = dynamic_cast<simpl::if_statement*>(stmt.get());
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
			const auto& stmt = ast[0];

			auto expr = dynamic_cast<simpl::expr_statement*>(stmt.get());
			Assert::IsNotNull(expr);
		}

		TEST_METHOD(TestParseSimplObjectEmpty)
		{
			auto ast = simpl::parse("object obj{}");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];

			auto expr = dynamic_cast<simpl::object_definition_statement*>(stmt.get());
			Assert::IsNotNull(expr);

			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::AreEqual(size_t{ 0 }, expr->members().size());

		}

		TEST_METHOD(TestParseSimpleObjectWMembers)
		{
			auto ast = simpl::parse("object obj { mem1; mem2; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];

			const auto expr = dynamic_cast<const simpl::object_definition_statement*>(stmt.get());
			Assert::IsNotNull(expr);
			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::AreEqual(size_t{ 2 }, expr->members().size());
		}

		TEST_METHOD(TestParseSimplObjectWInitializer)
		{
			auto ast = simpl::parse("object obj { mem1=1; mem2=\"king\"; }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];

			const auto expr = dynamic_cast<const simpl::object_definition_statement*>(stmt.get());
			Assert::IsNotNull(expr);

			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::AreEqual(size_t{ 2 }, expr->members().size());
		}

		TEST_METHOD(TestParseSimpleObjectInherits)
		{
			auto ast = simpl::parse("object obj inherits obj2 { }");
			Assert::AreEqual(size_t{ 1 }, ast.size());
			const auto& stmt = ast[0];

			const auto expr = dynamic_cast<const simpl::object_definition_statement*>(stmt.get());
			Assert::IsNotNull(expr);

			Assert::AreEqual(std::string("obj"), expr->type_name());
			Assert::IsTrue(expr->inherits().has_value());
			Assert::AreEqual(std::string("obj2"), expr->inherits().value());
		}

		TEST_METHOD(TestParseSimplExplosionOperator)
		{
			auto ast = simpl::parse("foo...;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
			Assert::IsNotNull(expr);
		}

		TEST_METHOD(TestParseFunctionAddress)
		{
			auto ast = simpl::parse("&foo;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
			Assert::IsNotNull(expr);

			// Blah. We need a visitor to test the expression here.
		}

		TEST_METHOD(TestParsePreIncrement)
		{
			auto ast = simpl::parse("++i;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
			Assert::IsNotNull(expr);
		}

		TEST_METHOD(TestParserNestedPreIncrement)
		{
			auto ast = simpl::parse("++foo.i;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
			Assert::IsNotNull(expr);
		}

		TEST_METHOD(TestParserPostIncrement)
		{
			auto ast = simpl::parse("i++;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
			Assert::IsNotNull(expr);
		}

		TEST_METHOD(TestParserNestedPostIncrement)
		{
			auto ast = simpl::parse("++foo.i;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
			Assert::IsNotNull(expr);
		}

		TEST_METHOD(TestParseIdentifierSinglePath)
		{
			auto ast = simpl::parse("p.x;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
		}

		TEST_METHOD(TestParseIdentifierMultiplePath)
		{
			auto ast = simpl::parse("p.x.y;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
		}

		TEST_METHOD(TestParseIdentiferSinglePathAssignment)
		{
			auto ast = simpl::parse("p.x = 42;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());
		}

		TEST_METHOD(TestParseIdnetifierMultiplePathAssignment)
		{
			auto ast = simpl::parse("p.x.y = 42;");
			const auto expr = dynamic_cast<const simpl::expr_statement*>(ast[0].get());

		}

		TEST_METHOD(TestParseImportDirectiveStatement)
		{
			auto ast = simpl::parse("@import gui");
			const auto is = dynamic_cast<const simpl::import_statement*>(ast[0].get());
			Assert::IsNotNull(is);
			Assert::AreEqual(std::string{ "gui" }, is->libname());
		}

		TEST_METHOD(TestParseLoadLibDirectiveStatement)
		{
			auto ast = simpl::parse("@loadlib \"path/to/lib.dll\"");
			const auto lls = dynamic_cast<const simpl::load_library_statement*>(ast[0].get());
		}

	};
}
