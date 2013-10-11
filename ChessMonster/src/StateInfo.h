#pragma once

#include "NodeType.h"
#include "Move.h"

namespace cm {

struct StateInfo
{
public:

	uint64_t id; //TODO make const?

	int depth;

	int score;

	Move bestMove;

	NodeType nodeType;
};

}
