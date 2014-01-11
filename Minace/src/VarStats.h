#pragma once

#include <cmath>

namespace mnc {

class VarStats
{
private:
	uint64_t mN = 0;
	double mSum = 0, mSumSqr = 0;
public:

	void add(double x)
	{
		mSum += x;
		mSumSqr += x * x;
		++mN;
	}

	uint64_t count() const
	{
		return mN;
	}

	double sum() const
	{
		return mSum;
	}

	double avg() const
	{
		return mSum / mN;
	}

	double stdDev() const
	{
		return std::sqrt((mSumSqr - mSum * mSum / mN) / (mN - 1));
	}

	double error() const
	{
		return 2 * stdDev() / std::sqrt(mN);
	}

	void reset()
	{
		*this = VarStats();
	}

	std::string toStr(unsigned precision) const
	{
		std::stringstream ss;
		ss << std::setprecision(precision) << std::fixed;
		ss << avg() << "\u00b1" << error();
		return ss.str();
	}
};

}
