#pragma once

#include "NodeType.h"
#include "Move.h"

namespace mnc {

struct StateInfo
{
public:

	uint64_t id;

	uint8_t depth;

	uint8_t age;

	int score;

	Move bestMove;

	NodeType nodeType;
};

}
