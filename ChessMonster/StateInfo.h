#pragma once

#include "Move.h"

namespace cm {

struct StateInfo
{
public:

	enum class NodeType : unsigned
	{
		NONE = 0,
		EXACT = 1,
		LOWER_BOUND = 2,
		UPPER_BOUND = 3,
	};

	uint64_t id; //TODO make const?

	int depth;

	int score;

	Move bestMove;

	NodeType nodeType;
};

}
