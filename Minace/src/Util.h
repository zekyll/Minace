#pragma once

#include <string>
#include <cstring>
#include <memory>
#include <cmath>

namespace mnc {

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

/**
 * Replaces all occurences of a in s with b.
 */
inline std::string stringReplace(const std::string& s, const std::string& a, const std::string& b)
{
	std::string r = s;

	if (!a.length())
		return r;

	size_t idx = 0;
	for (;; idx += b.length()) {
		idx = r.find(a.c_str(), idx);
		if (idx == std::string::npos)
			break;
		r.replace(idx, a.length(), b);
	}

	return r;
}

/* Rounds value up to nearest power of 2. */
unsigned long long roundUpToPowerOfTwo(unsigned long long x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
	x |= x >> 32;
    x++;
    return x;
}

}
