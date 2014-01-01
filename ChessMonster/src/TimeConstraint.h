
#include "Player.h"

#pragma once

namespace cm {

/**
 * Structure for holding the parameters that control the length and depth of search. There are 4
 * different constraints that can be used independently and combined in arbitrary ways.
 * 1) Search depth
 * 2) Time
 * 3) Node count
 * 4) Standard chess clocks with time left, increment and moves left.
 * If no constraints are specified the search is infinite and must be stopped by explicity calling
 * stop(), or if the AI decides to stop itself (e.g. when finds a guaranteed mate).
 */
class TimeConstraint
{
public:

	// Limit by search depth. 0 if not used.
	unsigned depth;

	// Limit search by time. 0 if not used.
	double time;

	// Limit by number of searched nodes. 0 if not used.
	unsigned long long nodes;

	// Time left in clock for each player. 0 if clock is not used.
	double clock[Player::COUNT];

	// Clock increment per move for each player.
	double clockIncrement[Player::COUNT];

	// Full moves until next time control. 0 if all moves must be done within the clock.
	unsigned clockMovesLeft;

	TimeConstraint()
	: TimeConstraint(0, 0, 0, 0, 0, 0, 0, 0)
	{
	}

	TimeConstraint(unsigned depth, double time = 0.0, unsigned long long nodes = 0)
	: TimeConstraint(0, 0, 0, 0, 0, depth, time, nodes)
	{
	}

	TimeConstraint(double whiteClock, double blackClock, double whiteClockIncrement,
			double blackClockIncrement, unsigned clockMovesLeft = 0,
			unsigned depth = 0, double time = 0, unsigned long long nodes = 0)
	: depth(depth), time(time), nodes(nodes), clockMovesLeft(clockMovesLeft)
	{
		clock[Player::WHITE] = whiteClock;
		clock[Player::BLACK] = blackClock;
		clockIncrement[Player::WHITE] = whiteClockIncrement;
		clockIncrement[Player::BLACK] = blackClockIncrement;
	}
};

}