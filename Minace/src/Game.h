#pragma once

#include "GameState.h"
#include "GameObserver.h"
#include "Player.h"
#include "GamePlayer.h"
#include "TimeConstraint.h"
#include <vector>
#include <chrono>

namespace mnc {

enum class GameResultType
{
	NORMAL, OUT_OF_TIME, ILLEGAL_MOVE, LOST_CONNECTION
};

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

	std::string mErrorMsg;

	GameResultType mResultType;

public:

	Game(GamePlayer& whitePlayer, GamePlayer& blackPlayer, const TimeConstraint& tc,
			const GameState& state = GameState(), GameObserver* observer = nullptr)
	: mState(state), mTimeConstraint(tc), mResult(Player::NONE), mObserver(observer),
	mResultType(GameResultType::NORMAL)
	{
		mPlayers[Player::WHITE] = &whitePlayer;
		mPlayers[Player::BLACK] = &blackPlayer;
	}

	void run()
	{
		init(Player::WHITE);
		init(Player::BLACK);

		while (!mState.isCheckMate() && !mState.isStaleMate()) {
			mResultType = getAndProcessMove();
			if (mResultType != GameResultType::NORMAL) {
				mResult = ~(mState.activePlayer());
				break;
			}
		}

		if (mState.isCheckMate())
			mResult = ~mState.activePlayer();

		if (mObserver)
			mObserver->notifyEnd(*this, mResult);
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

	const TimeConstraint& timeConstraint() const
	{
		return mTimeConstraint;
	}

	const std::string& errorMsg() const
	{
		return mErrorMsg;
	}

	GameResultType resultType() const
	{
		return mResultType;
	}

private:

	bool init(Player player)
	{
		try {
			mPlayers[player]->startNewGame();
			return true;
		} catch (std::exception& e) {
			mErrorMsg = e.what();
			mResult = ~player;
			mResultType = GameResultType::LOST_CONNECTION;
			return false;
		}
	}

	GameResultType getAndProcessMove()
	{
		Player player = mState.activePlayer();

		auto t1 = std::chrono::high_resolution_clock::now();
		mTimeConstraint.startTurn(player);

		Move move;
		try {
			move = mPlayers[player]->getMove(mState, mTimeConstraint);
		} catch (std::exception& e) {
			mErrorMsg = e.what();
			return GameResultType::LOST_CONNECTION;
		}

		auto t2 = std::chrono::high_resolution_clock::now();
		double t = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() * 1e-6;
		if (!mTimeConstraint.endTurn(player, t))
			return GameResultType::OUT_OF_TIME;

		mMoves.push_back(move);
		if (!mState.isLegalMove(move))
			return GameResultType::ILLEGAL_MOVE;
		mState.makeMove(move);
		if (mObserver)
			mObserver->notifyMove(*this, mMoves.size() - 1, *mPlayers[player], move, t);

		return GameResultType::NORMAL;
	}
};

}
