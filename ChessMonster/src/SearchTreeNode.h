#pragma once

#include "NodeType.h"
#include "Move.h"
#include "StateInfo.h"
#include <string>
#include <vector>

namespace cm {

/**
 * Stores information about a search tree node. Does not affect the behavior of the search
 * algorithm, and is just used for providing information about the search.
 */
class SearchTreeNode
{
private:
	std::vector<SearchTreeNode> mNodes;

	int mPly;

	int mAlpha;

	int mBeta;

	Player mPlayer;

	Move mMove;

	int mScore;

	NodeType mNodeType;

public:

	SearchTreeNode()
	: SearchTreeNode(0, 0, 0, Player::NONE, Move())
	{
	}

	SearchTreeNode(int ply, int alpha, int beta, Player player, Move move)
	: mPly(ply), mAlpha(alpha), mBeta(beta), mPlayer(player), mMove(move), mScore(0),
	mNodeType(NodeType::NONE)
	{
	}

	SearchTreeNode(SearchTreeNode&& rhs) = default;

	SearchTreeNode& operator=(SearchTreeNode&& rhs) = default;

	void setResult(int score, NodeType nodeType)
	{
		mScore = score;
		mNodeType = nodeType;
	}

	std::string toStr() const
	{
		std::string str;
		if (mMove) {
			str = mMove.toStr();
		} else {
			str = mPly > 0 ? "Null move search" : "";
		}
		if (mPly > 0) {
			str += " " + getIneqSign(-mScore, -mBeta, -mAlpha) + itostr(-mScore);
		}
		return str
				+ " (\u03b1=" + itostr(mAlpha)
				+ " \u03b2=" + itostr(mBeta)
				+ " s" + getIneqSign(mScore, mAlpha, mBeta) + itostr(mScore)
				+ ")";
	}

	std::vector<SearchTreeNode>& nodes()
	{
		return mNodes;
	}

	const std::vector<SearchTreeNode>& nodes() const
	{
		return mNodes;
	}

	int ply() const
	{
		return mPly;
	}

	int alpha() const
	{
		return mAlpha;
	}

	int beta() const
	{
		return mBeta;
	}

	Player player() const
	{
		return mPlayer;
	}

	Move move() const
	{
		return mMove;
	}

	int score() const
	{
		return mScore;
	}

	NodeType nodeType() const
	{
		return mNodeType;
	}

private:

	static std::string itostr(int x)
	{
		if (x == Scores::MIN)
			return "-\u221e";
		else if (x == Scores::MIN + 1)
			return "-\u221e+1";
		else if (x == Scores::MAX)
			return "\u221e";
		else if (x == Scores::MAX - 1)
			return "\u221e-1";
		else
			return std::to_string(x);
	}

	static std::string getIneqSign(int score, int alpha, int beta)
	{
		if (score <= alpha)
			return "\u2264";
		else if (score >= beta)
			return "\u2265";
		else
			return "=";
	}
};

}
