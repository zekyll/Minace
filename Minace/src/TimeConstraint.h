
#include "Player.h"

#pragma once

namespace mnc {

/**
 * Structure for holding the parameters that control the length and depth of search. There are 4
 * different constraints that can be used independently and combined in arbitrary ways.
 * 1) Search depth
 * 2) Time
 * 3) Node count
 * 4) Standard chess clocks with time left, increment (using Fischer delay) and moves left.
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

	// Initial clock value. 0 if clock is not used.
	double clockInitialValue[Player::COUNT];

	// Time left in clock for each player. 0 if clock is not used.
	double clock[Player::COUNT];

	// Clock increment per move for each player.
	double clockIncrement[Player::COUNT];

	// Initial value of clockMovesLeft.
	unsigned clockMovesInitialValue;

	// Full moves until next time control. 0 if all moves must be done within the clock amount.
	// When this reaches 0 it is set back to its initial value and clock is incremented by its
	// initial value.
	unsigned clockMovesLeft;

	TimeConstraint()
	: TimeConstraint(0, 0, 0, 0, 0, 0)
	{
	}

	TimeConstraint(unsigned depth, double time = 0.0, unsigned long long nodes = 0)
	: TimeConstraint(0, 0, 0, depth, time, nodes)
	{
	}

	TimeConstraint(double clock, double clockIncrement, unsigned clockMovesLeft = 0,
			unsigned depth = 0, double time = 0, unsigned long long nodes = 0)
	: depth(depth), time(time), nodes(nodes), clockMovesInitialValue(clockMovesLeft),
	clockMovesLeft(clockMovesLeft)
	{
		clockInitialValue[Player::WHITE] = clock;
		clockInitialValue[Player::BLACK] = clock;
		this->clock[Player::WHITE] = clock;
		this->clock[Player::BLACK] = clock;
		this->clockIncrement[Player::WHITE] = clockIncrement;
		this->clockIncrement[Player::BLACK] = clockIncrement;
	}

	/* Increment clock in start of turn (Fischer delay). */
	void startTurn(Player player)
	{
		clock[player] += clockIncrement[player];
	}

	/* Decrement clock in the end of turn and check whether out of time. */
	bool endTurn(Player player, double time)
	{
		if (clock[player]) {
			clock[player] -= time;
			if (clock[player] <= 0)
				return false;
		}
		return true;
	}

	void endFullTurn()
	{
		if (clockMovesLeft) {
			if (--clockMovesLeft == 0) {
				clock[Player::WHITE] += clockInitialValue[Player::WHITE];
				clock[Player::BLACK] += clockInitialValue[Player::BLACK];
				clockMovesLeft = clockMovesInitialValue;
			}
		}
	}
};

}
