#pragma once

#include "Move.h"

namespace cm {

struct StateInfo
{
public:
	static constexpr unsigned NODE_TYPE_EXACT = 0;

	static constexpr unsigned NODE_TYPE_LOWER_BOUND = 1;

	static constexpr unsigned NODE_TYPE_UPPER_BOUND = 2;

	const uint64_t id;

	unsigned depth;

	int score;

	Move bestMove;

	int nodeType;

	StateInfo(uint64_t id)
	: id(id)
	{
	}
};

}
