#pragma once

#include "Game.h"
#include "ExternalUciEngine.h"
#include "VarStats.h"
#include <ostream>
#include <vector>
#include <mutex>
#include <thread>
#include <fstream>
#include <unordered_map>

namespace cm {
/* Stores UCI traffic and automatically parses debug values from info strings. */
class EngineLogger : public UciLogger
{
private:

	std::stringstream mLines;

	std::unordered_map<std::string, double> mDebugValues;

public:

	virtual void onOutput(const std::string& msg) override
	{
		std::stringstream ss;
		ss << msg;
		std::string token;
		ss >> token;
		if (token == "info" && ss >> token && token == "string"
				&& ss >> token && token == "debug") {
			double x;
			while (ss >> token >> x)
				mDebugValues[token] = x;
		}
		mLines << msg << std::endl;
	}

	virtual void onInput(const std::string& msg) override
	{
		mLines << ">> " << msg << std::endl;
	}

	void appendToFile(const std::string& filename)
	{
		std::ofstream ofs(filename, std::ios_base::out | std::ios_base::app);
		ofs << mLines.str();
	}

	double debugValue(const std::string& name) const
	{
		auto r = mDebugValues.find(name);
		return r != mDebugValues.end() ? r->second : 0;
	}
};

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

	std::string mOutputDir;

	size_t mThreadCount;

	double mTime, mInc;

	TimeConstraint mTimeConstraint;

	// Statistics about games with each player. (Index 0 is for total values.)
	std::vector<VarStats> mMatchStats;

	std::mutex mMutex;

	unsigned mMatchCount;

	bool mLogEngines;

public:

	Tournament(const std::string& tournamentFile, std::ostream& out)
	: mOut(out), mThreadCount(1), mTime(3.0), mInc(0.1), mMatchCount(50), mLogEngines(false)
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
			} else if (cmd == "outputdir") {
				ss >> mOutputDir;
			} else if (cmd == "log_engines") {
				ss >> mLogEngines;
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

		reset();

		unsigned matchesLeft = (mMatchCount + 1) / 2 * 2;
		unsigned matchesPlayed = 0;
		std::vector<std::thread> threads;
		for (size_t threadIdx = 0; threadIdx < mThreadCount; ++threadIdx) {
			threads.emplace_back([this, threadIdx, &matchesLeft, &matchesPlayed] {

				for (;;) {
					size_t opponentIdx;
					{
						std::lock_guard<std::mutex> l(mMutex);
						if (!matchesLeft)
							break;
						matchesLeft -= 2;
						opponentIdx = 1 + matchesPlayed % (mExecutableFileNames.size() - 1);
						matchesPlayed += 2;
					}

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

	void reset()
	{
		// Reset stats.
		mMatchStats.clear();
		mMatchStats.resize(mExecutableFileNames.size());

		// Clear log files.
		if (mLogEngines) {
			for (size_t i = 0; i < mExecutableFileNames.size(); ++i)
				std::ofstream ofs(mOutputDir + "/E" + std::to_string(i) + ".txt");
		}
	}

	void addResult(size_t opponentIdx, double score, size_t threadIdx)
	{
		std::lock_guard<std::mutex> l(mMutex);

		mMatchStats[opponentIdx].add(0.5 * score);

		if (opponentIdx > 0) {
			mOut << "T" << threadIdx << ": E0 vs E" << opponentIdx << ": ";
			mOut << std::setprecision(1) << std::fixed;
			mOut << score << "/2" << std::endl;
		} else if (mMatchStats[opponentIdx].count() % PRINT_INTERVAL == 0) {
			printStats();
		}
	}

	void printStats()
	{
		mOut << std::endl;
		for (size_t i = 0; i < mExecutableFileNames.size(); ++i) {
			if (i > 0)
				mOut << "Vs E" << i << ":  ";
			else
				mOut << "Total:  ";
			mOut << std::setprecision(1) << std::fixed;
			mOut << (2 * mMatchStats[i].sum()) << "/" << (mMatchStats[i].count() * 2) << ", "
					<< std::setprecision(3)
					<< "Avg " << mMatchStats[i].toStr(3)
					<< std::endl;
		}
		mOut << std::endl;
	}

	double playMatchPair(size_t opponentIdx)
	{
		EngineLogger elog1, elog2;
		ExternalUciEngine p1(mExecutableFileNames[0],{}, mLogEngines ? &elog1 : nullptr, "E0");
		ExternalUciEngine p2(mExecutableFileNames[opponentIdx],{}, mLogEngines ?  &elog2 : nullptr,
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

		if (mLogEngines) {
			elog1.appendToFile(mOutputDir + "/E0.txt");
			elog2.appendToFile(mOutputDir + "/E" + std::to_string(opponentIdx) + ".txt");
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
//		std::lock_guard<std::mutex> l(mMutex);
//
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