#pragma once

#include "../src/TreeGenerator.h"
#include "../ttest/ttest.h"
#include <memory>
#include <cstddef>
#include <memory>

namespace cm {

class TreeGeneratorTest : public ttest::TestBase
{
private:

	std::unique_ptr<TreeGenerator> tg;

	TTEST_BEFORE()
	{
		tg.reset(new TreeGenerator(1));
	}

	TTEST_CASE("New tree is empty.")
	{
		TTEST_EQUAL(tg->getTree().nodes().size(), 0U);
	}

	TTEST_CASE("Saves node info.")
	{
		tg->startNode(10, 11, Player::WHITE, Move("Ka8xKa7"));
		tg->endNode(12, NodeType::UPPER_BOUND);
		SearchTreeNode tree = tg->getTree();
		TTEST_EQUAL(tree.alpha(), 10);
		TTEST_EQUAL(tree.beta(), 11);
		TTEST_EQUAL(tree.player(), Player::WHITE);
		TTEST_EQUAL(tree.move().toStr(), Move("Ka8xKa7").toStr());
		TTEST_EQUAL(tree.score(), 12);
		TTEST_EQUAL((int) tree.nodeType(), (int) NodeType::UPPER_BOUND);
	}

	TTEST_CASE("Generates correct tree with multiple children.")
	{
		tg->startNode(1, 0, Player::NONE, Move());
		tg->startNode(2, 0, Player::NONE, Move());
		tg->endNode(2, NodeType::NONE);
		tg->startNode(3, 0, Player::NONE, Move());
		tg->endNode(3, NodeType::NONE);
		tg->endNode(1, NodeType::NONE);
		SearchTreeNode tree = tg->getTree();
		TTEST_EQUAL(tree.alpha(), 1);
		TTEST_EQUAL(tree.score(), 1);
		TTEST_EQUAL(tree.nodes().size(), 2u);
		TTEST_EQUAL(tree.nodes()[0].alpha(), 2);
		TTEST_EQUAL(tree.nodes()[0].score(), 2);
		TTEST_EQUAL(tree.nodes()[1].alpha(), 3);
		TTEST_EQUAL(tree.nodes()[1].score(), 3);
	}

	TTEST_CASE("Is limited by max depth.")
	{
		tg->startNode(1, 0, Player::NONE, Move());
		tg->startNode(2, 0, Player::NONE, Move());
		tg->startNode(3, 0, Player::NONE, Move());
		tg->endNode(3, NodeType::NONE);
		tg->endNode(2, NodeType::NONE);
		tg->endNode(1, NodeType::NONE);
		SearchTreeNode tree = tg->getTree();
		TTEST_EQUAL(tree.ply(), 0);
		TTEST_EQUAL(tree.alpha(), 1);
		TTEST_EQUAL(tree.score(), 1);
		TTEST_EQUAL(tree.nodes().size(), 1u);
		TTEST_EQUAL(tree.nodes()[0].ply(), 1);
		TTEST_EQUAL(tree.nodes()[0].alpha(), 2);
		TTEST_EQUAL(tree.nodes()[0].score(), 2);
		TTEST_EQUAL(tree.nodes()[0].nodes().size(), 0u);
	}

	TTEST_CASE("Starts a new tree after ending root node.")
	{
		tg->startNode(1, 0, Player::NONE, Move());
		tg->endNode(1, NodeType::NONE);
		tg->startNode(2, 0, Player::NONE, Move());
		tg->endNode(2, NodeType::NONE);
		SearchTreeNode tree = tg->getTree();
		TTEST_EQUAL(tree.alpha(), 2);
		TTEST_EQUAL(tree.score(), 2);
	}
};

}
