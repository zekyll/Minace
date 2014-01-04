#pragma once

#include "Move.h"
#include "Process.h"
#include "GameState.h"
#include "TimeConstraint.h"
#include "Scores.h"
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <cmath>

namespace cm {

/*
 * Allows basic interaction with an external chess engine that uses UCI protocol.
 */
class ExternalUciEngine : public GamePlayer
{
private:

	Process mEngine;

	std::ostream* mLog;

	int mScore;

	std::string mName;

public:

	explicit ExternalUciEngine(const std::string& path, const std::vector<std::string>& args
			= std::vector<std::string>(), std::ostream* log = nullptr,
			const std::string name = "")
	: mEngine(path, args), mLog(log), mScore(0), mName(name)
	{
		mEngine.in().exceptions(std::ios::badbit | std::ios::failbit);
		mEngine.out().exceptions(std::ios::badbit | std::ios::failbit);

		writeLine("uci");
		std::string line;
		while (readLine(line)) {
			if (line == "uciok")
				break;
		}

		setOptions();
	}

	virtual void startNewGame() override
	{
		writeLine("ucinewgame");
	};

	virtual Move getMove(const GameState& state, const TimeConstraint& tc) override
	{
		writeLine("position fen " + state.toStr());

		writeLine("go" + getTimeControlCommands(tc));

		mScore = 0;

		std::string line;
		while (readLine(line)) {
			std::stringstream ss;
			ss << line;
			std::string cmd;
			ss >> cmd;
			if (cmd == "info") {
				readInfo(ss);
			} else if (cmd == "bestmove") {
				std::string moveStr;
				ss >> moveStr;
				try {
					return Move(moveStr, state.board());
				} catch (std::exception& e) {
					throw std::runtime_error("Illegal move " + moveStr);
				}
			}
		}

		return Move();
	}

	virtual int getScore() override
	{
		return mScore;
	}

	virtual void stop() override
	{
		writeLine("stop");
	};

	virtual void quit() override
	{
		writeLine("quit");
		mEngine.wait();
	};

	virtual std::string name() const override
	{
		return mName;
	}

	virtual ~ExternalUciEngine()
	{
		try {
			mEngine.wait();
		} catch (...) {
		}
	}

private:

	void readInfo(std::stringstream& ss)
	{
		std::string info;
		while (ss >> info) {
			if (info == "score") {
				ss >> info;
				if (info == "cp") {
					ss >> mScore;
				} else if (info == "mate") {
					ss >> mScore;
					mScore = Scores::getCheckMateScore(mScore);
				}
			}
		}
	}

	void setOptions()
	{
		// TODO

		writeLine("isready");
		std::string line;
		while (readLine(line)) {
			if (line == "readyok")
				break;
		}
	}

	std::string getTimeControlCommands(const TimeConstraint& tc)
	{
		std::string r;
		if (tc.nodes)
			r += " nodes " + std::to_string(tc.nodes);
		if (tc.depth)
			r += " depth " + std::to_string(tc.depth);
		if (tc.time)
			r += " movetime " + std::to_string(std::lround(1000.0 * tc.time));
		if (tc.clockMovesLeft)
			r += " movestogo " + std::to_string(tc.clockMovesLeft);
		if (tc.clock[Player::WHITE])
			r += " wtime " + std::to_string(std::lround(1000.0 * tc.clock[Player::WHITE]));
		if (tc.clock[Player::BLACK])
			r += " btime " + std::to_string(std::lround(1000.0 * tc.clock[Player::BLACK]));
		if (tc.clockIncrement[Player::WHITE])
			r += " winc " + std::to_string(std::lround(1000.0 * tc.clockIncrement[Player::WHITE]));
		if (tc.clockIncrement[Player::BLACK])
			r += " binc " + std::to_string(std::lround(1000.0 * tc.clockIncrement[Player::BLACK]));
		if (tc.clockMovesLeft)
			r += " movestogo " + std::to_string(tc.clockMovesLeft);
		return r;
	}

	std::istream& readLine(std::string& line)
	{
		std::getline(mEngine.out(), line);
		if (mLog)
			*mLog << "< " << line << std::endl;
		return mEngine.out();
	}

	void writeLine(const std::string& line)
	{
		mEngine.in() << line << std::endl;
		if (mLog)
			*mLog << ">>> " << line << std::endl;
	}
};

}
