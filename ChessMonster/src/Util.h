#pragma once

#include <string>
#include <cstring>

namespace cm {

template<typename ...TArgs>
std::string strFormat(size_t len, const char* fmt, TArgs ...args)
{
	std::unique_ptr<char[] > buf(new char[len]);
	std::snprintf(buf.get(), len, fmt, args...);
	return buf.get();
}

/**
 * Calculates the branching factor of a uniform tree with given node count and depth. Uses Newton's
 * method to solve the equation: totalNodes = (1 - bf ^ (depth + 1)) / (1 - bf)
 */
inline double calculateEffectiveUniformBranchingFactor(double totalNodes, unsigned depth)
{
	double bf = pow(totalNodes, 1.0 / depth), dbf = 0;
	do {
		double g = totalNodes * (1 - bf) + pow(bf, depth + 1) - 1;
		double dg = -totalNodes + (depth + 1) * pow(bf, depth);
		dbf = -g / dg;
		bf += dbf;
	} while (std::abs(dbf) > 1e-10);
	return bf;
}

}
