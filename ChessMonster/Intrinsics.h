#pragma once

namespace cm {

inline unsigned bitCount(unsigned long long x)
{
	return __builtin_popcountll(x);
}

inline unsigned countTrailingZeros(unsigned long long x)
{
	return __builtin_ctzll(x);
}

inline unsigned long long lowestOneBit(unsigned long long x)
{
	return 1ULL << __builtin_ctzll(x);
}

}
