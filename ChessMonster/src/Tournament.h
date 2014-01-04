#pragma once

#include "Game.h"
#include "ExternalUciEngine.h"
#include <ostream>
#include <vector>
#include <random>
#include <mutex>
#include <thread>
#include <fstream>

namespace cm {

/*
 * Plays one UCI engine against other engines using random starting positions. Tournament
 * parameters are loaded from configuration file. To reduce variance matches are played in pairs
 * where each position is played twice, flipping player colors between matches.
 */
class Tournament : private GameObserver
{
private:

	static constexpr unsigned PRINT_INTERVAL = 25;

	std::ostream& mOut;

	std::vector<std::string> mExecutableFileNames;

	size_t mThreadCount;

	double mTime, mInc;

	TimeConstraint mTimeConstraint;

	struct MatchStatistics
	{
		unsigned n = 0;
		double sum = 0, sumSqr = 0;
	};

	// Statistics about games with each player. (Index 0 is for total values.)
	std::vector<MatchStatistics> mStats;

	std::mutex mMutex;

	unsigned mMatchCount;

public:

	Tournament(const std::string& tournamentFile, std::ostream& out)
	: mOut(out), mThreadCount(1), mTime(3.0), mInc(0.1), mMatchCount(50)
	{
		std::ifstream ifs(tournamentFile);
		std::string line;
		while (std::getline(ifs, line)) {
			if (line.empty() || line[0] == '#')
				continue;

			std::stringstream ss;
			ss << line;
			std::string cmd;
			ss >> cmd;

			if (cmd == "threads") {
				ss >> mThreadCount;
			} else if (cmd == "time") {
				ss >> mTime;
			} else if (cmd == "inc") {
				ss >> mInc;
			} else if (cmd == "matches") {
				ss >> mMatchCount;
			} else if (cmd == "engine") {
				std::string tmp;
				ss >> tmp;
				mExecutableFileNames.push_back(tmp);
			}
		}
		mTimeConstraint = TimeConstraint(mTime, mInc);
	}

	std::vector<Move> moves[32];

	void run()
	{
		mOut << "=== Tournament ===" << std::endl;
		mOut << "Thread count: " << mThreadCount << std::endl;
		mOut << "Clock: " << mTime << std::endl;
		mOut << "Clock increment: " << mInc << std::endl;
		mOut << "Matches: " << mMatchCount << std::endl;
		mOut << "Engines: " << std::endl;
		for (size_t i = 0; i < mExecutableFileNames.size(); ++i) {
			mOut << "E" << i << ": " << mExecutableFileNames[i] << std::endl;
		}
		mOut << std::endl;

		mStats.clear();
		mStats.resize(mExecutableFileNames.size());

		unsigned matchesLeft = (mMatchCount + 1) / 2 * 2;
		std::vector<std::thread> threads;
		for (size_t threadIdx = 0; threadIdx < mThreadCount; ++threadIdx) {
			threads.emplace_back([this, threadIdx, &matchesLeft] {
				std::random_device rd;
				std::mt19937_64 rng(rd());
				std::uniform_int_distribution<size_t> distr(1, mExecutableFileNames.size() - 1);

				for (;;) {
					{
						std::lock_guard<std::mutex> l(mMutex);
						if (!matchesLeft)
							break;
						matchesLeft -= 2;
					}

					size_t opponentIdx = distr(rng);
					double r = playMatchPair(opponentIdx);
					addResult(opponentIdx, r, threadIdx);
					addResult(0, r, threadIdx);
				}
			});
		}
		for (std::thread& th : threads)
			th.join();

		if (mMatchCount % PRINT_INTERVAL != 0)
			printStats();

		mOut << "Done." << std::endl;
	}

private:

	void addResult(size_t opponentIdx, double score, size_t threadIdx)
	{
		std::lock_guard<std::mutex> l(mMutex);

		mStats[opponentIdx].sum += score;
		mStats[opponentIdx].sumSqr += score * score;
		++mStats[opponentIdx].n;

		if (opponentIdx > 0) {
			mOut << "T" << threadIdx << ": E0 vs E" << opponentIdx << ": ";
			mOut << std::setprecision(1) << std::fixed;
			mOut << score << "/2" << std::endl;
		} else if (mStats[opponentIdx].n % PRINT_INTERVAL == 0) {
			printStats();
		}
	}

	void printStats()
	{
		mOut << std::endl;
		for (size_t i = 0; i < mExecutableFileNames.size(); ++i) {
			double& sum = mStats[i].sum;
			double& sumSqr = mStats[i].sumSqr;
			unsigned& n = mStats[i].n;

			double avg = 0.5 * sum / n;
			double stdev = 0.5 * sqrt((sumSqr - sum * sum / n) / (n - 1));

			if (i > 0)
				mOut << "Vs E" << i << ":  ";
			else
				mOut << "Total:  ";
			mOut << std::setprecision(1) << std::fixed;
			mOut << sum << "/" << (n * 2) << ", "
					<< std::setprecision(3)
					<< "Avg " << avg << " +- " << 2 * stdev / sqrt(n)
					<< std::endl;
		}
		mOut << std::endl;
	}

	double playMatchPair(size_t opponentIdx)
	{
		ExternalUciEngine p1(mExecutableFileNames[0],{}, nullptr, "E0");
		ExternalUciEngine p2(mExecutableFileNames[opponentIdx],{}, nullptr,
				"E" + std::to_string(opponentIdx));

		GameState state = generateRandomOpening(6);
		//mOut << state.toStr(true) << std::endl;
		double score = playMatch(state, p1, p2);
		score += 1 - playMatch(state, p2, p1);

		try {
			p1.quit();
		} catch (std::exception& e) {
			mOut << "Failed to close " << p1.name() << ": " << e.what() << std::endl;
		}
		try {
			p2.quit();
		} catch (std::exception& e) {
			mOut << "Failed to close " << p2.name() << ": " << e.what() << std::endl;
		}

		return score;
	}

	double playMatch(const GameState& state, GamePlayer& white, GamePlayer& black)
	{
		Game game(white, black, mTimeConstraint, state, this);
		game.run();

		if (game.result() == Player::WHITE)
			return 1.0;
		else if (game.result() == Player::BLACK)
			return 0.0;
		else
			return 0.5;
	}

	virtual void notifyMove(Game& game, unsigned ply, GamePlayer& player,
			Move move, double time) override
	{
//		mOut << ply / 2 + 1 << ". " << (ply % 2 == 1 ? "... " : "")
//				<< move.toStr()
//				<< " " << Scores::toStr(player.getScore())
//				<< " " << std::setprecision(3) << std::fixed << time
//				<< " (" << game.timeConstraint().clock[Player::WHITE] << ", "
//				<< game.timeConstraint().clock[Player::BLACK] << ", "
//				<< game.timeConstraint().clockMovesLeft << ")"
//				<< std::endl;
	}

	virtual void notifyEnd(Game& game, Player result) override
	{
		std::lock_guard<std::mutex> l(mMutex);

		if (game.resultType() == GameResultType::NORMAL)
			return;

		Player loser = ~game.result();
		std::string name = game.player(loser).name();

		if (game.resultType() == GameResultType::ILLEGAL_MOVE) {
			mOut << "Illegal move by " << name << ": " << game.moves().back().toStr()
					<< ". (" << game.state() << ")" << std::endl;
		} else if (game.resultType() == GameResultType::LOST_CONNECTION) {
			mOut << "Connection was lost to " << name << ": "
					<< game.errorMsg() << std::endl;
		} else if (game.resultType() == GameResultType::OUT_OF_TIME) {
			mOut << name << " ran out of time. " << std::endl;
		}
	}

	GameState generateRandomOpening(unsigned depth)
	{
		std::mt19937_64 rng(std::random_device{}
		());

		GameState state;
		std::vector<Move> moves;
		for (unsigned i = 0; i < depth; ++i) {
			moves.clear();
			state.getLegalMoves(moves);
			std::uniform_int_distribution<unsigned> distr(0, moves.size() - 1);
			for (;;) {
				Move move = moves[distr(rng)];
				state.makeMove(move);
				if (!state.isCheckMate() && !state.isStaleMate())
					break;
				state.undoMove(move);
			}
		}

		return state;
	}
};

}