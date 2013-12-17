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
};

}