#pragma once

#include "Sqr.h"
#include "Mask.h"
#include "Move.h"
#include "Piece.h"
#include "BitBoard.h"
#include "MoveMasks.h"
#include "Zobrist.h"
#include <string>

namespace cm {

template<typename T>
class GameState_t
{
public:

	struct HistoryEntry
	{
		uint64_t zobristCode;
		Mask castlingRights;
		Sqr enPassantSqr;
		unsigned halfMoveClock;
	};

private:
	BitBoard mBoard;

	Player mPlayer;

	unsigned mPly;

	std::vector<HistoryEntry> mHist;

public:

	GameState_t()
	: GameState_t(BitBoard("Ra1 Nb1 Bc1 Qd1 Ke1 Bf1 Ng1 Rh1 a2 b2 c2 d2 e2 f2 g2 h2",
	"Ra8 Nb8 Bc8 Qd8 Ke8 Bf8 Ng8 Rh8 a7 b7 c7 d7 e7 f7 g7 h7"),
	Player::WHITE)
	{
	}

	GameState_t(std::string whitePieces, std::string blackPieces, Player startingPlayer)
	: GameState_t(BitBoard(whitePieces, blackPieces), startingPlayer)
	{
	}

	GameState_t(const BitBoard& board, Player startingPlayer)
	: mBoard(board), mPlayer(startingPlayer), mPly(0), mHist(1)
	{

		mHist[0].zobristCode = Zobrist::EMPTY_RND;
		mHist[0].enPassantSqr = Sqr::NONE;
		if (startingPlayer == Player::BLACK)
			mHist[0].zobristCode ^= Zobrist::PLAYER_RND;

		if (mBoard(Player::WHITE, Piece::KING, Sqr(60))) {
			checkCastlingRight(Player::WHITE, Sqr(56));
			checkCastlingRight(Player::WHITE, Sqr(63));
		}
		if (mBoard(Player::BLACK, Piece::KING, Sqr(4))) {
			checkCastlingRight(Player::BLACK, Sqr(0));
			checkCastlingRight(Player::BLACK, Sqr(7));
		}

		for (unsigned player = 0; player < Player::COUNT; ++player) {
			for (unsigned sqr = 0; sqr < Sqr::COUNT; ++sqr) {
				Piece piece = mBoard.getPieceType(Player(player), Sqr(sqr));
				if (piece)
					mHist[0].zobristCode ^= Zobrist::PIECE_SQR_RND[player][piece][sqr];
			}
		}
	}

	Player activePlayer() const
	{
		return mPlayer;
	}

	const BitBoard& board() const
	{
		return board;
	}

	unsigned ply() const
	{
		return mPly;
	}

	Sqr enPassantSqr() const
	{
		return mHist[mPly].enPassantSqr;
	}

	Mask castlingRights() const
	{
		return mHist[mPly].castlingRights;
	}

	unsigned getHalfMoveClock() const
	{
		return mHist[mPly].halfMoveClock;
	}

	void makeMove(Move move)
	{
		growArrays();
		mHist[mPly + 1].zobristCode = mHist[mPly].zobristCode;
		++mPly;
		updateHalfMoveClock(move);
		if (move.isCapture())
			removeCapturedPiece(move);
		handleCastlingMove(move);
		updateEnPassantSquare(move);
		updateCastlingRights(move);
		removePiece(mPlayer, move.pieceType(), move.fromSqr());
		addPiece(mPlayer, move.newType(), move.toSqr());
		changeNextMovingPlayer();
	}

	void undoMove(Move move)
	{
		changeNextMovingPlayer();
		removePiece(mPlayer, move.newType(), move.toSqr());
		addPiece(mPlayer, move.pieceType(), move.fromSqr());
		undoCastlingMove(move);
		if (move.isCapture())
			restoreCapturedPiece(move);
		--mPly;
	}

	void makeNullMove()
	{
		growArrays();
		mHist[mPly + 1].zobristCode = mHist[mPly].zobristCode;
		++mPly;
		mHist[mPly].halfMoveClock = mHist[mPly - 1].halfMoveClock;
		if (mHist[mPly - 1].enPassantSqr)
			mHist[mPly].zobristCode ^= Zobrist::EN_PASSANT_RND[mHist[mPly - 1].enPassantSqr];
		mHist[mPly].enPassantSqr = Sqr::NONE;
		mHist[mPly].castlingRights = castlingRights[mPly - 1];
		changeNextMovingPlayer();
	}

	void undoNullMove()
	{
		changeNextMovingPlayer();
		--mPly;
	}

//	std::vector<Move> getLegalMoves()
//	{
//		std::vector<Move> moves;
//		moves.reserve(256);
//		for (Sqr fromSqr : mBoard(mPlayer)) {
//			std::vector<Move> sqrMoves = getLegalMoves(fromSqr);
//			moves.insert(moves.end(), sqrMoves.begin(), sqrMoves.end());
//		}
//		return moves;
//	}
	void getLegalMoves(std::vector<Move>& moves)
	{
		for (Sqr fromSqr : mBoard(mPlayer))
			getLegalMoves(fromSqr, moves);
	}

	
//	std::vector<Move> getLegalMoves(Sqr fromSqr)
//	{
//		std::vector<Move> moves;
//		moves.reserve(27);
//
//		Mask movesMask = getPseudoLegalMoves(mPlayer, fromSqr);
//
//		for (Sqr toSqr : movesMask) {
//			Piece pieceType = mBoard.getPieceType(mPlayer, fromSqr);
//			Piece capturedType = mBoard.getPieceType(~mPlayer, toSqr);
//			if (toSqr == mHist[mPly].enPassantSqr && pieceType == Piece::PAWN)
//				capturedType = Piece::PAWN;
//			Move move(fromSqr, toSqr, pieceType, capturedType, pieceType);
//			if (!isLegalMove(move))
//				continue;
//			if (pieceType == Piece::PAWN && toSqr.row() == mPlayer * 7) {
//				for (int promoType = Piece::QUEEN; promoType <= Piece::KNIGHT; ++promoType)
//					moves.emplace_back(fromSqr, toSqr, pieceType, capturedType, Piece(promoType));
//			} else
//				moves.push_back(move);
//		}
//
//		return moves;
//	}

	void getLegalMoves(Sqr fromSqr, std::vector<Move>& moves)
	{
		Mask movesMask = getPseudoLegalMoves(mPlayer, fromSqr);

		for (Sqr toSqr : movesMask) {
			Piece pieceType = mBoard.getPieceType(mPlayer, fromSqr);
			Piece capturedType = mBoard.getPieceType(~mPlayer, toSqr);
			if (toSqr == mHist[mPly].enPassantSqr && pieceType == Piece::PAWN)
				capturedType = Piece::PAWN;
			Move move(fromSqr, toSqr, pieceType, capturedType, pieceType);
			if (!isLegalMove(move))
				continue;
			if (pieceType == Piece::PAWN && toSqr.row() == mPlayer * 7) {
				for (unsigned promoType = Piece::QUEEN; promoType <= Piece::KNIGHT; ++promoType)
					moves.emplace_back(fromSqr, toSqr, pieceType, capturedType, Piece(promoType));
			} else
				moves.push_back(move);
		}
	}	
	
	Mask getPseudoLegalMoves(Player player, Sqr fromSqr) const
	{
		for (unsigned piece = 0; piece < Piece::COUNT; ++piece) {
			if (mBoard(mPlayer, Piece(piece), fromSqr))
				return getPseudoLegalMoves(player, Piece(piece), fromSqr);
		}

		return 0;
	}

	Mask getPseudoLegalMoves(Player player, Piece pieceType, Sqr fromSqr) const
	{
		Mask moves = 0;

		if (pieceType != Piece::PAWN) {
			moves = getThreatenedSquares(player, pieceType, fromSqr) & ~mBoard(player);
			if (pieceType == Piece::KING)
				moves |= getCastlingMoves(player);
			return moves;
		}

		unsigned row = fromSqr.row();
		unsigned col = fromSqr.col();
		unsigned nextRow = row - 1 + 2 * player;

		if ((nextRow & ~7) == 0) {
			if (!mBoard(Sqr(nextRow * 8 + col))) {
				moves |= Sqr(nextRow * 8 + col);
				Sqr doublePushSqr((4 - player) * 8 + col);
				if (row == 6 - 5 * player && !mBoard(doublePushSqr))
					moves |= doublePushSqr;
			}
			Mask enemySqrs = mBoard(~player);
			if (mHist[mPly].enPassantSqr)
				enemySqrs |= mHist[mPly].enPassantSqr;
			if (col > 0 && (enemySqrs & Sqr(nextRow * 8 + col - 1)))
				moves |= Sqr(nextRow * 8 + col - 1);
			if (col < 7 && (enemySqrs & Sqr(nextRow * 8 + col + 1)))
				moves |= Sqr(nextRow * 8 + col + 1);
		}

		return moves;
	}

	Mask getThreatenedSquares(Player player, Piece piece, Sqr fromSqr) const
	{
		Mask moves = 0;

		switch (piece)
		{
		case Piece::KING:
			moves |= MoveMasks::KING_MOVES[fromSqr];
			break;
		case Piece::QUEEN:
			moves |= MoveMasks::getQueenMoves(fromSqr, mBoard());
			break;
		case Piece::ROOK:
			moves |= MoveMasks::getRookMoves(fromSqr, mBoard());
			break;
		case Piece::BISHOP:
			moves |= MoveMasks::getBishopMoves(fromSqr, mBoard());
			break;
		case Piece::KNIGHT:
			moves |= MoveMasks::KNIGHT_MOVES[fromSqr];
			break;
		case Piece::PAWN:
			unsigned row = fromSqr.row();
			unsigned col = fromSqr.col();
			unsigned nextRow = row - 1 + 2 * player;
			if (!(nextRow & ~7)) {
				if (col > 0)
					moves |= Sqr(nextRow * 8 + col - 1);
				if (col < 7)
					moves |= Sqr(nextRow * 8 + col + 1);
			}
			break;
		}

		return moves;
	}

	bool isCheckMate()
	{
		for (Sqr sqr : mBoard(mPlayer)) {
			if (!getLegalMoves(sqr).empty())
				return false;
		}
		return isKingChecked(mPlayer);
	}

	bool isStaleMate()
	{
		if (isRepeatedState() || mHist[mPly].halfMoveClock >= 50)
			return true;
		for (Sqr sqr : mBoard(mPlayer)) {
			if (!getLegalMoves(sqr).empty())
				return false;
		}
		return !isKingChecked(mPlayer);
	}

	bool isKingChecked(Player defendingPlayer) const
	{
		Mask kingMask = mBoard(defendingPlayer, Piece::KING);
		return isSquareThreatened(defendingPlayer, kingMask);
	}

	bool isSquareThreatened(Player defendingPlayer, Mask sqrs) const
	{
		Player attckPlayer = ~defendingPlayer;
		for (unsigned piece = 0; piece < Piece::COUNT; ++piece) {
			Mask pieces = mBoard(attckPlayer, Piece(piece));
			for (Sqr attackingSqr : pieces) {
				Mask attackMoves = getThreatenedSquares(attckPlayer, Piece(piece), attackingSqr);
				if (sqrs & attackMoves)
					return true;
			}
		}
		return false;
	}

	uint64_t getId() const
	{
		return mHist[mPly].zobristCode;
	}

	bool operator==(const GameState_t& rhs) const
	{
		bool result = mBoard == rhs.mBoard
				&& mPlayer == rhs.mPlayer
				&& mHist[mPly].enPassantSqr == rhs.mHist[rhs.mPly].enPassantSqr
				&& mHist[mPly].castlingRights == rhs.mHist[rhs.mPly].castlingRights;
		assert(!result || mHist[mPly].zobristCode == rhs.zobristCode[rhs.mPly]);
		return result;
	}

	std::vector<Mask> getEarlierStates()
	{
		return std::vector<Mask > (mHist.begin(), mHist.begin() + mPly);
	}

	void changeNextMovingPlayer()
	{
		mPlayer = ~mPlayer;
		mHist[mPly].zobristCode ^= Zobrist::PLAYER_RND;
	}

	void addPiece(Player player, Piece piece, Sqr sqr)
	{
		mBoard.addPiece(player, piece, sqr);
		mHist[mPly].zobristCode ^= Zobrist::PIECE_SQR_RND[player][piece][sqr];
	}

	void removePiece(Player player, Piece piece, Sqr sqr)
	{
		mBoard.removePiece(player, piece, sqr);
		mHist[mPly].zobristCode ^= Zobrist::PIECE_SQR_RND[player][piece][sqr];
	}

	bool isLegalMove(Move move)
	{
		Player player = mPlayer;
		makeMove(move);
		bool legal = !isKingChecked(player);
		undoMove(move);
		return legal;
	}

	void removeCapturedPiece(Move move)
	{
		assert(move.isCapture());
		Sqr toSqr = move.toSqr();
		if (move.pieceType() == Piece::PAWN && toSqr == mHist[mPly - 1].enPassantSqr)
			removePiece(~mPlayer, Piece::PAWN, Sqr(toSqr + 8 - 16 * mPlayer));
		else
			removePiece(~mPlayer, move.capturedType(), toSqr);
	}

	void restoreCapturedPiece(Move move)
	{
		assert(move.isCapture());
		Sqr toSqr = move.toSqr();
		if (move.pieceType() == Piece::PAWN && toSqr == mHist[mPly - 1].enPassantSqr)
			addPiece(~mPlayer, Piece::PAWN, Sqr(toSqr + 8 - 16 * mPlayer));
		else
			addPiece(~mPlayer, move.capturedType(), toSqr);
	}

	void updateEnPassantSquare(Move move)
	{
		if (mHist[mPly - 1].enPassantSqr)
			mHist[mPly].zobristCode ^= Zobrist::EN_PASSANT_RND[mHist[mPly - 1].enPassantSqr];
		if (move.pieceType() == Piece::PAWN
				&& move.fromSqr() >> 3 == 6 - 5 * mPlayer
				&& move.toSqr() >> 3 == 4 - mPlayer) {
			mHist[mPly].enPassantSqr = Sqr(move.fromSqr() - 8 + 16 * mPlayer);
			mHist[mPly].zobristCode ^= Zobrist::EN_PASSANT_RND[mHist[mPly].enPassantSqr];
		} else
			mHist[mPly].enPassantSqr = Sqr::NONE;
	}

	void updateCastlingRights(Move move)
	{
		mHist[mPly].castlingRights = mHist[mPly - 1].castlingRights;
		if (move.pieceType() == Piece::KING) {
			removeCastlingRight(Sqr(56 * ~mPlayer));
			removeCastlingRight(Sqr(56 * ~mPlayer + 7));
		} else if (move.pieceType() == Piece::ROOK)
			removeCastlingRight(move.fromSqr());
		else if (move.isCapture())
			removeCastlingRight(move.toSqr());
	}

	void removeCastlingRight(Sqr rookSqr)
	{
		Mask sqrBit(rookSqr);
		if ((mHist[mPly].castlingRights & sqrBit) != 0) {
			mHist[mPly].castlingRights &= ~sqrBit;
			mHist[mPly].zobristCode ^= Zobrist::CASTLINGRIGHTS_RND[rookSqr];
		}
	}

	void handleCastlingMove(Move move)
	{
		Sqr toSqr = move.toSqr();
		if (move.pieceType() == Piece::KING && ((move.fromSqr() - toSqr) & 3) == 2) {
			Sqr rookFromSqr, rookToSqr;
			if (toSqr.col() == 2) {
				rookFromSqr = Sqr(8 * toSqr.row() + 0);
				rookToSqr = Sqr(8 * toSqr.row() + 3);
			} else {
				rookFromSqr = Sqr(8 * toSqr.row() + 7);
				rookToSqr = Sqr(8 * toSqr.row() + 5);
			}
			addPiece(mPlayer, Piece::ROOK, rookFromSqr);
			removePiece(mPlayer, Piece::ROOK, rookToSqr);
		}
	}

	void undoCastlingMove(Move move)
	{
		Sqr toSqr = move.toSqr();
		if (move.pieceType() == Piece::KING && ((move.fromSqr() - toSqr) & 3) == 2) {
			Sqr rookFromSqr, rookToSqr;
			if (toSqr.col() == 2) {
				rookFromSqr = Sqr(8 * toSqr.row() + 0);
				rookToSqr = Sqr(8 * toSqr.row() + 3);
			} else {
				rookFromSqr = Sqr(8 * toSqr.row() + 7);
				rookToSqr = Sqr(8 * toSqr.row() + 5);
			}
			removePiece(mPlayer, Piece::ROOK, rookToSqr);
			addPiece(mPlayer, Piece::ROOK, rookFromSqr);
		}
	}

	Mask getCastlingMoves(Player player) const
	{
		unsigned rowOffset = 56 * ~player;
		Mask moves;
		if (mHist[mPly].castlingRights & Sqr(rowOffset)) {
			Mask betweenSquares = (1ULL << 1 | 1ULL << 2 | 1ULL << 3) << rowOffset;
			if (!(betweenSquares & mBoard())) {
				Mask kingSqrs = (1ULL << 2 | 1ULL << 3 | 1ULL << 4) << rowOffset;
				if (!isSquareThreatened(player, kingSqrs))
					moves |= Sqr(2 + rowOffset);
			}
		}
		if (mHist[mPly].castlingRights & Sqr(rowOffset + 7)) {
			Mask betweenSquares = (1ULL << 5 | 1ULL << 6) << rowOffset;
			if (!(betweenSquares & mBoard())) {
				Mask kingSqrs = (1ULL << 4 | 1ULL << 5 | 1ULL << 6) << rowOffset;
				if (!isSquareThreatened(player, kingSqrs))
					moves |= Sqr(6 + rowOffset);
			}
		}
		return moves;
	}

	void growArrays()
	{
		if (mPly + 1 >= mHist.size())
			mHist.resize(2 * mHist.size());
	}

	bool isRepeatedState() const
	{
		for (unsigned i = mPly - mHist[mPly].halfMoveClock; i < mPly; ++i) {
			if (mHist[i].zobristCode == mHist[mPly].zobristCode)
				return true;
		}
		return false;
	}

	void updateHalfMoveClock(Move move)
	{
		if (move.isCapture() || move.pieceType() == Piece::PAWN)
			mHist[mPly].halfMoveClock = 0;
		else
			mHist[mPly].halfMoveClock = mHist[mPly - 1].halfMoveClock + 1;
	}

	void checkCastlingRight(Player player, Sqr sqr)
	{
		if (mBoard(player, Piece::ROOK, sqr)) {
			mHist[mPly].castlingRights |= sqr;
			mHist[mPly].zobristCode ^= Zobrist::CASTLINGRIGHTS_RND[sqr];
		}
	}
};

typedef GameState_t<void> GameState;

}