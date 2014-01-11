#pragma once

#include "GamePlayer.h"
#include "Move.h"
#include "Player.h"

namespace mnc {

class Game;

/*
 * Callback class that is used to get notifications about Game events.
 */
class GameObserver
{
public:

	virtual void notifyMove(Game& game, unsigned ply, GamePlayer& player, Move move, double time)
	{
	}

	virtual void notifyEnd(Game& game, Player result)
	{
	}

	virtual ~GameObserver()
	{
	}
};

}
