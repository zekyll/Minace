#pragma once

namespace cm {

/*
 * Callback class that is used to get notifications about Game events.
 */
class GameObserver
{
public:

	virtual void notifyMove(const GameState& state, unsigned ply, GamePlayer& player, Move move,
			double time)
	{
	}

	virtual void notifyEnd(const GameState& state, Player result)
	{
	}

	virtual ~GameObserver()
	{
	}
};

}
