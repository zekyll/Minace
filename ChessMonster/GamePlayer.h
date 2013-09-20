#pragma once

#include "GameState.h"
#include "Move.h"

namespace cm{

class GamePlayer
{
public:
	virtual Move getMove(const GameState& state) = 0;
};

}
