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
class Uci
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
	: mIn(in), mOut(out), mLog(log)
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
			mOut << "uciok" << std::endl;
		} else if (cmd == "debug") {

		} else if (cmd == "setoption") {

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
			mLog << "Unknown command" << std::endl;
		}

		return true;
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
				Move move = parseMove(moveStr, mPosition.board());
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
		double wtime = 0, btime = 0, winc = 0, binc = 0, moveTime = 0;
		int depth = 99;
		long long nodes = -1;
		int movesToGo = 0;
		bool infinite = false/*, ponder = false, mate = false*/;

		// Read go parameters
		std::string cmd;
		while (ss >> cmd) {
			if (cmd == "wtime") {
				ss >> wtime;
			} else if (cmd == "btime") {
				ss >> btime;
			} else if (cmd == "winc") {
				ss >> winc;
			} else if (cmd == "binc") {
				ss >> binc;
			} else if (cmd == "movetime") {
				ss >> moveTime;
			} else if (cmd == "infinite") {
				infinite = true;
			} else if (cmd == "movestogo") {
				ss >> movesToGo;
			} else if (cmd == "depth") {
				ss >> depth;
			} else if (cmd == "nodes") {
				ss >> nodes;
			} else if (cmd == "searchmoves") {
				ss >> moveTime;
			} else if (cmd == "ponder") {
				//ponder = true;
			} else if (cmd == "mate") {
				//mate = true;
			}
		}

		// Time control
		if (!movesToGo) {
			movesToGo = 30;
		}
		if (infinite) {
			moveTime = 0.0;
		} else {
			int time = mPosition.activePlayer() == Player::WHITE ? wtime : btime;
			int inc = mPosition.activePlayer() == Player::WHITE ? winc : binc;
			if (time == 0.0)
				time = 100.0;
			time += inc * movesToGo;
			moveTime = time / (movesToGo + 1);
		}

		// Wait for previous thread to end.
		cleanup();

		// Create AI
		mLog << "Time: " << 0.001 * moveTime << std::endl;
		auto cb = std::bind(&Uci::infoCallback, this, std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4);
		mAi.reset(new MinMaxAI(cb, depth, 30, 0.001 * moveTime, 0));
		mStartTime = std::chrono::high_resolution_clock::now();

		mAiThread.reset(new std::thread([this]() {
			GameState mStateCopy = mPosition; // Copy in case mState is modified during getMove
			Move bestMove = mAi->getMove(mStateCopy);

			mOut << "bestmove " << moveToStr(bestMove) << std::endl;
			mLog << "Best move: " << bestMove.toStr() << std::endl;
		}));
	}

	void infoCallback(int depth, int score, long long nodes, const std::vector<Move>& pv)
	{
		mOut << "info";
		mOut << " depth " << depth;
		mOut << " score cp " << score;
		mOut << " nodes " << nodes;
		auto t = std::chrono::high_resolution_clock::now();
		unsigned ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - mStartTime).count();
		mOut << " time " << ms;
		//mOut << " nps " << ?;
		mOut << " pv";
		for (Move m : pv)
			mOut << " " << moveToStr(m);
		mOut << std::endl;
	}

	static std::string moveToStr(Move move)
	{
		std::string r = move.fromSqr().toStr() + move.toSqr().toStr();
		if (move.isPromotion())
			r += (char) std::tolower(move.newType().toStr()[0]);
		return r;
	}

	Move parseMove(const std::string& s, const BitBoard& board)
	{
		size_t i = 0;

		// From
		Sqr fromSqr(s, i);
		Piece pieceType = board.getPieceType(fromSqr);

		// To
		Sqr toSqr(s, i);
		Piece capturedType = board.getPieceType(toSqr);

		// En passant
		if (pieceType == Piece::PAWN && fromSqr.row() != toSqr.row() && !capturedType)
			capturedType = Piece::PAWN;

		// Promotion type
		Piece newType = pieceType;
		if (i != s.length())
			newType = s.substr(i++, 1);

		return Move(fromSqr, toSqr, pieceType, capturedType, newType);
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
