#pragma once

#include "MinMaxAI.h"
#include "BitBoard.h"
#include "GameState.h"
#include "Move.h"
#include <sstream>
#include <memory>
#include <thread>

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
		mPosition = GameState();
		std::string startpos, movesCmd, moveStr;
		ss >> startpos >> movesCmd;

		mLog << "Moves:";
		while (ss >> moveStr) {
			Move move = parseMove(moveStr, mPosition.board());
			mLog << " " << move.toStr();
			mPosition.makeMove(move);
		}
		mLog << std::endl;

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
		mAi.reset(new MinMaxAI(nullptr, depth, 30, 0.001 * moveTime, 0));

		mAiThread.reset(new std::thread([this]() {
			GameState mStateCopy = mPosition; // Copy in case mState is modified during getMove
			Move bestMove = mAi->getMove(mStateCopy);

			mOut << "bestmove " << bestMove.fromSqr().toStr() << bestMove.toSqr().toStr();
			mLog << "Best move: " << bestMove.toStr() << std::endl;
			if (bestMove.isPromotion())
				mOut << (char) std::tolower(bestMove.newType().toStr()[0]);
			mOut << std::endl;
		}));
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
