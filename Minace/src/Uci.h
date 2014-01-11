#pragma once

#include "MinMaxAI.h"
#include "Epd.h"
#include "BitBoard.h"
#include "GameState.h"
#include "Move.h"
#include <sstream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

namespace cm {

/**
 * Implementation of Universal Chess Interface.
 */
class Uci : public InfoCallback
{
private:

	std::istream& mIn;

	std::ostream& mOut, & mLog;

	GameState mPosition;

	std::unique_ptr<MinMaxAI> mAi;

	std::unique_ptr<std::thread> mAiThread;

	std::chrono::high_resolution_clock::time_point mStartTime;

public:

	Uci(std::istream& in, std::ostream& out, std::ostream& log)
	: mIn(in), mOut(out), mLog(log), mAi(new MinMaxAI(this))
	{
	}

	void run()
	{
		mLog << "Started" << std::endl;

		bool running = true;
		while (running) {
			std::string line;
			std::getline(mIn, line);
			mLog << ">>> " << line << std::endl;
			running = handleCommand(line);
		}

		cleanup();
	}

private:

	bool handleCommand(const std::string& line)
	{
		std::stringstream ss;
		ss << line;
		std::string cmd;
		ss >> cmd;

		if (cmd == "uci") {
			mOut << "id name ChessMonster" << std::endl;
			mOut << "id author Zekyll" << std::endl;
			mOut << "option name Hash type spin default 32 min 1 max 8192" << std::endl;
			mOut << "uciok" << std::endl;
		} else if (cmd == "debug") {

		} else if (cmd == "setoption") {
			setOption(ss);
		} else if (cmd == "isready") {
			mOut << "readyok" << std::endl;
		} else if (cmd == "register") {

		} else if (cmd == "ucinewgame") {

		} else if (cmd == "position") {
			position(ss);
		} else if (cmd == "go") {
			go(ss);
		} else if (cmd == "stop") {
			if (mAi)
				mAi->stop();
		} else if (cmd == "ponderhit") {

		} else if (cmd == "quit") {
			return false;
		} else {
			if (!mAi || !mAi->cmd(line))
				mLog << "Unknown command" << std::endl;
		}

		return true;
	}

	void setOption(std::stringstream& ss)
	{
		std::string s1, name, s2;
		ss >> s1 >> name >> s2;
		if (s1 != "name" || s2 != "value")
			return;
		if (name == "Hash") {
			unsigned value;
			ss >> value;
			value = std::max(1u, std::min(value, 8192u));
			mAi.reset(new MinMaxAI(this, value * (1ull << 20)));
		}
	}

	void position(std::stringstream& ss)
	{
		// Starting position is either "startpos" or FEN string.
		std::string startPos;
		ss >> startPos;
		if (startPos == "startpos") {
			mPosition = GameState();
		} else if (startPos == "fen") {
			ss >> startPos;
			std::string tmp;
			for (int i = 0; i < 5; ++i) {
				ss >> tmp;
				startPos += " " + tmp;
			}
			mPosition = GameState(Epd(startPos));
		} else {
			mPosition = GameState();
		}

		std::string movesCmd, moveStr;
		if (ss >> movesCmd && movesCmd == "moves") {
			mLog << "Moves:";
			while (ss >> moveStr) {
				Move move(moveStr, mPosition.board());
				mLog << " " << move.toStr();
				mPosition.makeMove(move);
			}
			mLog << std::endl;
		}

		mLog << "Position:" << std::endl << mPosition << std::endl;
	}

	void go(std::stringstream& ss)
	{
		// Parameters
		TimeConstraint tc;
		bool infinite = false/*, ponder = false, mate = false*/;

		auto getTime = [&](double& val) {
			ss >> val;
			val *= 0.001;
		};

		// Read go parameters
		std::string cmd;
		while (ss >> cmd) {
			if (cmd == "wtime") {
				getTime(tc.clock[Player::WHITE]);
			} else if (cmd == "btime") {
				getTime(tc.clock[Player::BLACK]);
			} else if (cmd == "winc") {
				getTime(tc.clockIncrement[Player::WHITE]);
			} else if (cmd == "binc") {
				getTime(tc.clockIncrement[Player::BLACK]);
			} else if (cmd == "movetime") {
				getTime(tc.time);
			} else if (cmd == "infinite") {
				infinite = true;
			} else if (cmd == "movestogo") {
				ss >> tc.clockMovesLeft;
			} else if (cmd == "depth") {
				ss >> tc.depth;
			} else if (cmd == "nodes") {
				ss >> tc.nodes;
			} else if (cmd == "searchmoves") {
				throw std::runtime_error("searchmoves not supported yet");
			} else if (cmd == "ponder") {
				//ponder = true;
			} else if (cmd == "mate") {
				//mate = true;
			}
		}

		if (infinite)
			tc = TimeConstraint();

		// Wait for previous thread to end.
		cleanup();

		mStartTime = std::chrono::high_resolution_clock::now();

		mAiThread.reset(new std::thread([this, tc]() {
			GameState mStateCopy = mPosition; // Copy in case mState is modified during getMove
			Move bestMove = mAi->getMove(mStateCopy, tc);

					mOut << "bestmove " << bestMove.toStr(true) << std::endl;
					mLog << "Best move: " << bestMove.toStr() << std::endl;
		}));
	}

	virtual void notifyPv(unsigned depth, int score, const std::vector<Move>& pv) override
	{
		mOut << "info";
		mOut << " depth " << depth;
		mOut << " score cp " << score;
		mOut << " pv";
		for (Move m : pv)
			mOut << " " << m.toStr(true);
		mOut << std::endl;
	}

	virtual void notifyIterDone(unsigned depth, int score, uint64_t nodes, size_t hashEntries,
			size_t hashCapacity) override
	{
		mOut << "info";
		mOut << " depth " << depth;
		mOut << " nodes " << nodes;
		auto t = std::chrono::high_resolution_clock::now();
		unsigned ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - mStartTime).count();
		mOut << " time " << ms;
		mOut << " hashfull " << (unsigned) (1000.0 * hashEntries / hashCapacity);
		mOut << std::endl;
	}

	virtual void notifyString(const std::string& s) override
	{
		mOut << "info string " << s << std::endl;
	}

	void cleanup()
	{
		if (mAi)
			mAi->stop();
		if (mAiThread) {
			mAiThread->join();
			mAiThread.reset();
		}
	}
};

}
