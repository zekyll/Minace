
#include "MinMaxAI.h"
#include "TranspositionTable.h"
#include "GameState.h"
#include "MoveList.h"
#include "Scores.h"
#include "Evaluator.h"
#include "Test.h"
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace cm;

vector<Move> moves[32];

uint64_t perft(GameState& state, unsigned depth)
{
	moves[depth].clear();
	state.getLegalMoves(moves[depth]);
	if (depth == 1)
		return moves[depth].size();
	uint64_t count = 0;
	for (Move m : moves[depth]) {
		state.makeMove(m);
		count += perft(state, depth - 1);
		state.undoMove(m);
	}
	return count;
}

template class Scores_t<int>;
template class MoveMasks_t<Mask>;
template class Zobrist_t<uint64_t>;

int main(int argc, char** argv)
{
	GameState state;

	//	auto moves = state.getLegalMoves();
	//	
	//	for (Move m : moves)
	//		cout << m.toStr() << endl;

//	for (int i = 1; i <= 6; ++i)
//		cout << perft(state, i) << endl;
	
	Test t;
	t.run();
//	
//	TranspositionTable<uint64_t> tt;
//	tt.put(123);
//	cout << tt.get(123) << endl;

	return 0;
}
