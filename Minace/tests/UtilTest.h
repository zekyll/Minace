#pragma once

#include "../src/Util.h"
#include "../ttest/ttest.h"
#include <cmath>

namespace cm {

class UtilTest : public ttest::TestBase
{
private:

	TTEST_CASE("calculateEffectiveUniformBranchingFactor returns right value.")
	{
		double eubf = calculateEffectiveUniformBranchingFactor(13918.6597, 8);
		TTEST_EQUAL(true, std::abs(eubf - 3.14159264) < 1e-4);
	}

	TTEST_CASE("stringReplace.")
	{
		TTEST_EQUAL(stringReplace("cabababab", "abab", "foo"), "cfoofoo");
	}

	TTEST_CASE("stringReplace with empty search string.")
	{
		TTEST_EQUAL(stringReplace("abc", "", "c"), "abc");
	}

	TTEST_CASE("roundUpToPowerOfTwo()")
	{
		TTEST_EQUAL(roundUpToPowerOfTwo(0), 0);
		TTEST_EQUAL(roundUpToPowerOfTwo(15), 16);
		TTEST_EQUAL(roundUpToPowerOfTwo(16), 16);
		TTEST_EQUAL(roundUpToPowerOfTwo(17), 32);
		TTEST_EQUAL(roundUpToPowerOfTwo(0x7fffffffffffffff), 0x8000000000000000);
		TTEST_EQUAL(roundUpToPowerOfTwo(0x8000000000000000), 0x8000000000000000);
		TTEST_EQUAL(roundUpToPowerOfTwo(0x8000000000000001), 0);
	}
};

}
