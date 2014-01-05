#pragma once

#include "GameState.h"
#include "Move.h"

namespace cm {

class GamePlayer
{
public:

	virtual void startNewGame()
	{
	};

	virtual Move getMove(const GameState& state, const TimeConstraint& tc) = 0;

	virtual std::string name() const
	{
		return "Player";
	}

	virtual int getScore()
	{
		return 0;
	}

	virtual void stop()
	{
	};

	virtual void quit()
	{
	};

	virtual bool cmd(const std::string& c)
	{
		return false;
	};

	virtual ~GamePlayer()
	{
	}
};

}
