#pragma once

#include "Move.h"
#include "Player.h"
#include "NodeType.h"
#include "SearchTreeNode.h"

namespace mnc {

/**
 * Saves first few levels of the search tree for debugging purposes etc.
 */
class TreeGenerator
{
private:
	unsigned mMaxDepth;

	SearchTreeNode mRoot;

	std::vector<SearchTreeNode*> mLevels;

	unsigned mPly;

public:

	TreeGenerator(unsigned maxDepth)
	: mMaxDepth(maxDepth), mPly(0)
	{
	}

	void clear()
	{
		mRoot.nodes().clear();
		mLevels.clear();
	}

	void startNode(int alpha, int beta, Player player, Move move)
	{
		if (mPly <= mMaxDepth) {
			if (mPly > 0) {
				mLevels.back()->nodes().emplace_back(mPly, alpha, beta, player, move);
				mLevels.push_back(&mLevels.back()->nodes().back());
			} else {
				mRoot = SearchTreeNode(mPly, alpha, beta, player, move);
				mLevels.push_back(&mRoot);
			}
		}
		++mPly;
	}

	void endNode(int score, NodeType nodeType)
	{
		if (mPly-- == mLevels.size()) {
			mLevels.back()->setResult(score, nodeType);
			mLevels.pop_back();
		}
	}

	SearchTreeNode getTree()
	{
		mPly = 0;
		return std::move(mRoot);
	}
};

}
