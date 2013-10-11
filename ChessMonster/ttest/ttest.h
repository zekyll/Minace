#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <sstream>

namespace ttest {

// Convert macro argument to string literal
#define TTEST_ARGTOSTR2(x) #x
#define TTEST_ARGTOSTR(x) TTEST_ARGTOSTR2(x)

// Concatenates two macro arguments
#define TTEST_CONCAT2(A,B) A##B
#define TTEST_CONCAT(A,B) TTEST_CONCAT2(A,B)

/*
 * Exception thrown on test failure. May also be thrown manually.
 */
class TestException : public std::exception
{
private:
	const std::string mMg;

public:

	TestException(const std::string& msg)
	: mMg(msg)
	{
		;
	}

	const char* what() const noexcept
	{
		return mMg.c_str();
	}
};

/*
 * Base class for unit tests. 
 */
class TestBase
{
private:

	struct TestCase
	{
		std::function<void() > func;
		std::string description;
	};

	std::vector<TestCase> mTestCases;
protected:
	TestBase() = default;

	void addTestCase(std::function<void() > func, const char* description)
	{
		mTestCases.push_back(TestCase{func, description});
	}

	template<typename T1, typename T2>
	static void testEqual(const char* expr, const T1& actual, const T2& expected)
	{
		if (actual != expected) {
			std::stringstream ss;
			ss << "\"" << expr << "\", expected \"" << expected
					<< "\", actual \"" << actual << "\"";
			throw TestException(ss.str());
		}
	}

	virtual void ttestBefore() { };
public:
	virtual ~TestBase() = default;

	void run()
	{
		size_t nr = 0, passCount = 0;
		for (TestCase& tc : mTestCases) {
			std::cout << "  #" << nr + 1 << ": " << tc.description << " ";
			std::cout.flush();
			try {
				ttestBefore();
				tc.func();
				std::cout << " [OK]" << std::endl;
				++passCount;
			} catch (std::exception& e) {
				std::cout << " [FAILED] (" << e.what() << ")" << std::endl;
				//				if (abortOnFailure)
				//					break;
			} catch (...) {
				std::cout << " [FAILED]. (Unknown exception.)" << std::endl;
				//				if (abortOnFailure)
				//					break;
			}
			++nr;
		}
		std::cout << "Tests passed: " << passCount << " / " << nr << std::endl;
	}
};

// Helper macro for creating a unique id.
#define TTEST_UNIQUE_TEST_ID TTEST_CONCAT(test,__LINE__)

// Assert that EXPR equals EXPECTED.
#define TTEST_EQUAL(EXPR, EXPECTED) \
	testEqual(TTEST_ARGTOSTR(EXPR), EXPR, EXPECTED)

// Declares a member function to be called before each test case. Must be followed by function body.
#define TTEST_BEFORE(DESCRIPTION) \
	virtual void ttestBefore() override

// Registers and declares a test case member function. Must be followed by function body.
#define TTEST_CASE(DESCRIPTION) \
	int TTEST_CONCAT(TTEST_UNIQUE_TEST_ID,_init) = \
	(addTestCase(std::bind(&Test::TTEST_UNIQUE_TEST_ID, this), DESCRIPTION), 0); \
	void TTEST_UNIQUE_TEST_ID()
}
