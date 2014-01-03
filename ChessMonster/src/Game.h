#pragma once

#include "GameState.h"
#include "GameObserver.h"
#include "Player.h"
#include "GamePlayer.h"
#include "TimeConstraint.h"
#include <vector>
#include <chrono>

namespace cm {

/*
 * Allows playing a game between two players and manages game rules and time control. An observer
 * can be registered that is notified about game events.
 */
class Game
{
private:
	GameState mState;

	GamePlayer* mPlayers[Player::COUNT];

	TimeConstraint mTimeConstraint;

	Player mResult;

	GameObserver* mObserver;

	std::vector<Move> mMoves;

public:

	Game(GamePlayer& whitePlayer, GamePlayer& blackPlayer, const TimeConstraint& tc,
			const GameState& state = GameState(), GameObserver* observer = nullptr)
	: mState(state), mTimeConstraint(tc), mResult(Player::NONE), mObserver(observer)
	{
		mPlayers[Player::WHITE] = &whitePlayer;
		mPlayers[Player::BLACK] = &blackPlayer;
	}

	void run()
	{
		mPlayers[Player::WHITE]->startNewGame();
		mPlayers[Player::BLACK]->startNewGame();

		while (!mState.isCheckMate() && !mState.isStaleMate()) {
			if (!getAndProcessMove()) {
				mResult = ~(mState.activePlayer());
				break;
			}
		}

		if (mState.isCheckMate())
			mResult = ~mState.activePlayer();

		if (mObserver)
			mObserver->notifyEnd(mState, mResult);
	}

	GamePlayer& player(Player player)
	{
		return *mPlayers[player];
	}

	const GameState& state() const
	{
		return mState;
	}

	Player result() const
	{
		return mResult;
	}

	const std::vector<Move>& moves() const
	{
		return mMoves;
	}

private:

	bool getAndProcessMove()
	{
		Player player = mState.activePlayer();

		auto t1 = std::chrono::high_resolution_clock::now();
		mTimeConstraint.startTurn(player);

		Move move = mPlayers[player]->getMove(mState, mTimeConstraint);

		auto t2 = std::chrono::high_resolution_clock::now();
		double t = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() * 1e-6;
		if (!mTimeConstraint.endTurn(player, t))
			return false;

		if (!mState.isLegalMove(move)) {
			//throw std::runtime_error("Illegal move " + mState.toStr(true) + " " + move.toStr());
			return false;
		}
		mState.makeMove(move);
		mMoves.push_back(move);
		if (mObserver)
			mObserver->notifyMove(mState, mMoves.size() - 1, *mPlayers[player], move, t);
		return true;
	}
};

}
