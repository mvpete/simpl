#include "pch.h"
#include "CppUnitTest.h"

#include <simpl/simpl.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/// <summary>
/// This is to enable the CPPUnitTestFramework to use token_types in assertions
/// </summary>
namespace Microsoft {
namespace VisualStudio {
namespace CppUnitTestFramework
{
	template<> inline std::wstring ToString<simpl::token_types>(const simpl::token_types &t)
	{
		std::wstringstream wss;
		// TODO: Better formatting w/ switch
		wss << "token_type: " << (int)t;
		return wss.str();
	}
}
}
}

namespace simpl_test
{
	TEST_CLASS(simpl_tokenizer_test)
	{
	public:

		/// <summary>
		/// Assert that tokenizer::peek returns the same token.
		/// </summary>
		TEST_METHOD(TestTokenizerPeek)
		{
			std::string text("foo + bar");
			simpl::tokenizer t{ text };

			auto t1 = t.peek();
			auto t2 = t.peek();

			Assert::AreEqual(t1.type, t2.type);
			Assert::AreEqual(t1.begin, t2.begin);
			Assert::AreEqual(t1.end, t2.end);
		}

		/// <summary>
		/// Assert that the tokenizer will tokenize a simple expression
		/// </summary>
		TEST_METHOD(TestTokenizerSimpleExpression)
		{
			std::string text("foo + bar");
			simpl::tokenizer t{text};

			auto t1 = t.next();
			auto t2 = t.next();
			auto t3 = t.next();

			Assert::AreEqual(simpl::token_types::identifier_token, t1.type);
			Assert::AreEqual(simpl::token_types::op, t2.type);
			Assert::AreEqual(simpl::token_types::identifier_token, t3.type);
		}

		/// <summary>
		/// Assert the explosion operator is detected.
		/// </summary>
		TEST_METHOD(TestTokenizerExplosionOperator)
		{
			std::string text("...");
			simpl::tokenizer t{ text };
			auto t1 = t.next();

			Assert::AreEqual(simpl::token_types::op, t1.type);
		}

		/// <summary>
		/// Tests the explosion operator on an identifier.
		/// </summary>
		TEST_METHOD(TestTokenizerExplosionOperatorIndentifier)
		{
			std::string text{ "foo..." };
			simpl::tokenizer t{ text };
			auto t1 = t.next();
			auto t2 = t.next();

			Assert::AreEqual(simpl::token_types::identifier_token, t1.type);
			Assert::AreEqual(std::string{ "foo" }, t1.to_string());
			Assert::AreEqual(simpl::token_types::op, t2.type);
			Assert::AreEqual(std::string{ "..." }, t2.to_string());
		}

		TEST_METHOD(TestTokenIdentifier)
		{
			std::string text{ "identifier" };
			simpl::tokenizer t{ text };
			auto t1 = t.next();

			Assert::AreEqual(simpl::token_types::identifier_token, t1.type);
			Assert::AreEqual(text, t1.to_string());

		}

		TEST_METHOD(TestTokenStringLiteral)
		{
			std::string text{ "\"string literal\"" };
			simpl::tokenizer t{ text };
			auto t1 = t.next();

			Assert::AreEqual(simpl::token_types::literal, t1.type);
			Assert::AreEqual(std::string { "string literal" }, t1.to_string());
		}

		TEST_METHOD(TestTokenNumber)
		{
			std::string text{ "42" };
			simpl::tokenizer t{ text };
			auto t1 = t.next();

			Assert::AreEqual(simpl::token_types::number, t1.type);
			Assert::AreEqual(text, t1.to_string());
		}

		TEST_METHOD(TestTokenParenthesis)
		{
		}

		TEST_METHOD(TestTokenSqBracket)
		{
		}

		TEST_METHOD(TestTokenBracket)
		{
		}

		TEST_METHOD(TestTokenComma)
		{
		}

		TEST_METHOD(TestTokenDirective)
		{
			std::string text{ "@" };
			simpl::tokenizer tknzr{ text };
			auto t = tknzr.next();
			Assert::AreEqual(simpl::token_types::directive, t.type);
		}
		TEST_METHOD(TestTokenAmpersand)
		{
			std::string text{ "&" };
			simpl::tokenizer t{ text };
			auto t1 = t.next();

			Assert::AreEqual(simpl::token_types::op, t1.type);
			Assert::AreEqual(text, t1.to_string());
		}

		TEST_METHOD(TestTokenAddressFunction)
		{
			std::string text{ "&foo" };
			simpl::tokenizer t{ text };
			auto t1 = t.next();
			auto t2 = t.next();

			Assert::AreEqual(simpl::token_types::op, t1.type);
			Assert::AreEqual(simpl::token_types::identifier_token, t2.type);
			Assert::AreEqual(std::string{ "foo" }, t2.to_string());
		}
		
	};
}
