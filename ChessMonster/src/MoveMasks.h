#pragma once

#include "Mask.h"
#include "Player.h"
#include "Intrinsics.h"
#include <vector>

namespace cm {

template<typename TMask>
class MoveMasks_t
{
public:
	static constexpr TMask INITIAL_CASTLING_RIGHTS = 0x8100000000000081ULL;

	static constexpr TMask PROMOTABLE[] = {0x000000000000FF00, 0x00FF000000000000};

	static TMask KING_MOVES[Sqr::COUNT];

	static TMask KNIGHT_MOVES[Sqr::COUNT];

private:
	static TMask ROOK_OCCUPANCY_MASKS[Sqr::COUNT];

	static constexpr unsigned ROOK_OCCUPANCY_BITS[] = {
		12, 11, 11, 11, 11, 11, 11, 12,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		11, 10, 10, 10, 10, 10, 10, 11,
		12, 11, 11, 11, 11, 11, 11, 12
	};

	static constexpr uint64_t ROOK_OCCUPANCY_MAGIC_MULTIPLIERS[] = {
		0X808000802A144000, 0XA040200010004000, 0X0880088020001000, 0X8680040801100080,
		0X02000490080200A0, 0X0300010008040002, 0X4880328001000200, 0X0200010200284084,
		0X0000800080204000, 0X0081004000802100, 0X0641801000A00084, 0X10430010010900A0,
		0X2009001100040801, 0X28A0800400020080, 0X0204001844290A10, 0X0209002100004082,
		0X0380004040002000, 0X2030004020084000, 0X0050808020001000, 0X0421818010004800,
		0X4901010010040800, 0X0202808002000400, 0XE140040008224110, 0X0000020000810044,
		0X0080004040002000, 0X900A004600210080, 0X04C1100280200080, 0X0482100280080080,
		0X0804000808004080, 0X088A000200080411, 0X0001002100042200, 0X10140B0600308044,
		0X0810904000800028, 0X0040002800601000, 0X0211004011002000, 0X2120204012000A00,
		0X0006800800800400, 0X0101000401000802, 0X8002021854001001, 0X1000801040801100,
		0X8940820041020025, 0X0800402010004004, 0X0261002000430011, 0X0090000C21010010,
		0X2145000408010010, 0X0006000400808002, 0X084100020005000C, 0X3000484B8C120009,
		0X0000402200810200, 0X2008201880400080, 0X2104420010248200, 0X001C480081900080,
		0X0001018800108500, 0X005A000280040080, 0X0220081081020400, 0X1000010408488200,
		0X3081020040221282, 0X0080248700124001, 0X0118400820001101, 0X002020400A00460E,
		0X4002012004108802, 0X10810004004A0829, 0X0220480082100104, 0X4800008400483102
	};


	static TMask ROOK_MOVES[Sqr::COUNT][4096];

	static TMask BISHOP_OCCUPANCY_MASKS[Sqr::COUNT];

	static constexpr unsigned BISHOP_OCCUPANCY_BITS[] = {
		6, 5, 5, 5, 5, 5, 5, 6,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 9, 9, 7, 5, 5,
		5, 5, 7, 7, 7, 7, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		6, 5, 5, 5, 5, 5, 5, 6
	};

	static constexpr uint64_t BISHOP_OCCUPANCY_MAGIC_MULTIPLIERS[] = {
		0X084084481A004010, 0X012084810205A444, 0X0110210841002002, 0X0029040103780080,
		0X0010882000001400, 0X00042208C0226000, 0X010C809888400804, 0X0001840104901404,
		0X8000411001410902, 0X000604810C210A00, 0X0800040102020440, 0X0006044104200008,
		0X00212405040C8090, 0X4006008805400808, 0X4040028221114000, 0X0000060111011000,
		0X000A004008014408, 0X9042000888080098, 0X0020800108030040, 0X0808001882004480,
		0X0005000820082400, 0X081A000108022200, 0X10A2098400A40480, 0X5004420200420888,
		0X1020440010100250, 0XC808054008010820, 0X0058140002002202, 0X0002002002008200,
		0X0011001001004000, 0X000812000C410080, 0X0A21040022008480, 0X0A8101280A060100,
		0X2110820801208880, 0X0384040440025004, 0X0001080202010400, 0X0808400820020200,
		0X8400408020020200, 0X4000900080410080, 0X0084014400005400, 0X0400808900120102,
		0X08040421040008C1, 0X0040820842802000, 0X00110040B2031006, 0X0000202011040811,
		0X8100400109002202, 0X4081010102000100, 0X0062101A02088081, 0X0008320052000049,
		0X2900920842404010, 0X2024450088201800, 0X0000610841100000, 0X0002000020884103,
		0X6000808420820000, 0X0040101092182001, 0X0420201400A48000, 0X0020380321012204,
		0X0800804400844000, 0X400A010082012010, 0X4200100090480802, 0X04800010102A0800,
		0X0200200020204120, 0X9406240820488088, 0X28A0200410408109, 0X1C04011001020880
	};

	static TMask BISHOP_MOVES[Sqr::COUNT][4096];

public:

	static TMask getRookMoves(Sqr fromSqr, TMask allPieces)
	{
		unsigned hash = rookOccupancyHash(fromSqr, allPieces);
		return ROOK_MOVES[fromSqr][hash];
	}

	static TMask getBishopMoves(Sqr fromSqr, TMask allPieces)
	{
		unsigned hash = bishopOccupancyHash(fromSqr, allPieces);
		return BISHOP_MOVES[fromSqr][hash];
	}

	static TMask getQueenMoves(Sqr fromSqr, TMask allPieces)
	{
		unsigned rhash = rookOccupancyHash(fromSqr, allPieces);
		unsigned bhash = bishopOccupancyHash(fromSqr, allPieces);
		TMask moves = ROOK_MOVES[fromSqr][rhash] | BISHOP_MOVES[fromSqr][bhash];
		return moves;
	}

private:

	static unsigned rookOccupancyHash(Sqr sqr, TMask allPieces)
	{
		uint64_t hash = (uint64_t) allPieces;
		hash &= (uint64_t) ROOK_OCCUPANCY_MASKS[sqr];
		hash *= ROOK_OCCUPANCY_MAGIC_MULTIPLIERS[sqr];
		hash >>= 64 - ROOK_OCCUPANCY_BITS[sqr];
		return (unsigned) hash;
	}

	static unsigned bishopOccupancyHash(Sqr sqr, TMask allPieces)
	{
		uint64_t hash = (uint64_t) allPieces;
		hash &= (uint64_t) BISHOP_OCCUPANCY_MASKS[sqr];
		hash *= BISHOP_OCCUPANCY_MAGIC_MULTIPLIERS[sqr];
		hash >>= 64 - BISHOP_OCCUPANCY_BITS[sqr];
		return (unsigned) hash;
	}

	static TMask generateRookOccupancyMask(Sqr sqr, unsigned row, unsigned col)
	{
		TMask rowMask = 0x7EULL << (row * 8);
		TMask colMask = 0x0001010101010100ULL << col;
		return (rowMask | colMask) & ~TMask(sqr);
	}

	static TMask generateBishopOccupancyMask(Sqr sqr, unsigned row, unsigned col)
	{
		TMask mask = 0;
		mask |= generateSlidingMoves(row, col, -1, -1, 0);
		mask |= generateSlidingMoves(row, col, -1, 1, 0);
		mask |= generateSlidingMoves(row, col, 1, -1, 0);
		mask |= generateSlidingMoves(row, col, 1, 1, 0);
		TMask borders = 0xFF818181818181FFULL;
		return mask & ~borders & ~TMask(sqr);
	}

	static void generateRookMoves(Sqr sqr, unsigned row, unsigned col)
	{
		std::vector<unsigned> bitPositions = getBitPositions(ROOK_OCCUPANCY_MASKS[sqr]);
		unsigned variationCount = 1 << bitCount((uint64_t) ROOK_OCCUPANCY_MASKS[sqr]);
#ifdef NDEBUG
		bool used[4096] = {};
#endif
		for (unsigned i = 0; i < variationCount; ++i) {
			TMask occupancy = getOccupancyVariation(bitPositions, i);
			unsigned hash = rookOccupancyHash(sqr, occupancy);
#ifdef NDEBUG
			assert(!used[hash]);
			used[hash] = true;
#endif
			ROOK_MOVES[sqr][hash] |= generateSlidingMoves(row, col, -1, 0, occupancy);
			ROOK_MOVES[sqr][hash] |= generateSlidingMoves(row, col, 0, -1, occupancy);
			ROOK_MOVES[sqr][hash] |= generateSlidingMoves(row, col, 0, 1, occupancy);
			ROOK_MOVES[sqr][hash] |= generateSlidingMoves(row, col, 1, 0, occupancy);
		}
	}

	static void generateBishopMoves(Sqr sqr, unsigned row, unsigned col)
	{
		std::vector<unsigned> bitPositions = getBitPositions(BISHOP_OCCUPANCY_MASKS[sqr]);
		unsigned variationCount = 1 << bitCount((uint64_t) BISHOP_OCCUPANCY_MASKS[sqr]);
#ifdef NDEBUG
		bool used[4096] = {};
#endif
		for (unsigned i = 0; i < variationCount; ++i) {
			TMask occupancy = getOccupancyVariation(bitPositions, i);
			unsigned hash = bishopOccupancyHash(sqr, occupancy);
#ifdef NDEBUD
			assert(!used[hash]);
			used[hash] = true;
#endif
			BISHOP_MOVES[sqr][hash] |= generateSlidingMoves(row, col, -1, -1, occupancy);
			BISHOP_MOVES[sqr][hash] |= generateSlidingMoves(row, col, -1, 1, occupancy);
			BISHOP_MOVES[sqr][hash] |= generateSlidingMoves(row, col, 1, -1, occupancy);
			BISHOP_MOVES[sqr][hash] |= generateSlidingMoves(row, col, 1, 1, occupancy);
		}
	}

	static TMask generateSlidingMoves(unsigned row, unsigned col, int dr, int dc, TMask occupancy)
	{
		TMask moves = 0;
		for (;;) {
			row += dr;
			col += dc;

			TMask move = getMove(row, col);
			if (!move)
				break;
			moves |= move;
			if (occupancy & move)
				break;
		}
		return moves;
	}

	static TMask getOccupancyVariation(const std::vector<unsigned>& bitPositions,
			unsigned variationIdx)
	{
		TMask occupancy = 0;
		for (unsigned i = 0; i < bitPositions.size(); ++i) {
			if (variationIdx & (1 << i))
				occupancy |= Sqr(bitPositions[i]);
		}
		return occupancy;
	}

	static std::vector<unsigned> getBitPositions(TMask mask)
	{
		std::vector<unsigned> bitPositions;
		for (Sqr sqr : mask)
			bitPositions.push_back(sqr);
		return bitPositions;
	}

	static TMask generateKingMoves(unsigned row, unsigned col)
	{
		TMask moves;
		moves |= getMove(row - 1, col - 1);
		moves |= getMove(row - 1, col);
		moves |= getMove(row - 1, col + 1);
		moves |= getMove(row, col - 1);
		moves |= getMove(row, col + 1);
		moves |= getMove(row + 1, col - 1);
		moves |= getMove(row + 1, col);
		moves |= getMove(row + 1, col + 1);
		return moves;
	}

	static TMask generateKnightMoves(unsigned row, unsigned col)
	{
		TMask moves;
		moves |= getMove(row - 2, col - 1);
		moves |= getMove(row - 2, col + 1);
		moves |= getMove(row - 1, col - 2);
		moves |= getMove(row - 1, col + 2);
		moves |= getMove(row + 2, col - 1);
		moves |= getMove(row + 2, col + 1);
		moves |= getMove(row + 1, col - 2);
		moves |= getMove(row + 1, col + 2);
		return moves;
	}

	static TMask getMove(unsigned row, unsigned col)
	{
		if (((row | col) & ~7) != 0)
			return TMask();
		return Sqr(row * 8 + col);
	}

private:

	MoveMasks_t()
	{
		for (unsigned sqri = 0; sqri < 64; ++sqri) {
			Sqr sqr(sqri);
			unsigned row = sqr.row();
			unsigned col = sqr.col();
			KING_MOVES[sqr] = generateKingMoves(row, col);
			KNIGHT_MOVES[sqr] = generateKnightMoves(row, col);
			ROOK_OCCUPANCY_MASKS[sqr] = generateRookOccupancyMask(sqr, row, col);
			generateRookMoves(sqr, row, col);
			BISHOP_OCCUPANCY_MASKS[sqr] = generateBishopOccupancyMask(sqr, row, col);
			generateBishopMoves(sqr, row, col);
		}
	}

	static MoveMasks_t sStaticInit;
};

template<typename TMask>
constexpr TMask MoveMasks_t<TMask>::INITIAL_CASTLING_RIGHTS;

template<typename TMask>
constexpr TMask MoveMasks_t<TMask>::PROMOTABLE[];

template<typename TMask>
TMask MoveMasks_t<TMask>::KING_MOVES[];

template<typename TMask>
TMask MoveMasks_t<TMask>::KNIGHT_MOVES[];

template<typename TMask>
TMask MoveMasks_t<TMask>::ROOK_OCCUPANCY_MASKS[];

template<typename TMask>
constexpr unsigned MoveMasks_t<TMask>::ROOK_OCCUPANCY_BITS[];

template<typename TMask>
constexpr uint64_t MoveMasks_t<TMask>::ROOK_OCCUPANCY_MAGIC_MULTIPLIERS[];

template<typename TMask>
TMask MoveMasks_t<TMask>::ROOK_MOVES[][4096];

template<typename TMask>
TMask MoveMasks_t<TMask>::BISHOP_OCCUPANCY_MASKS[];

template<typename TMask>
constexpr unsigned MoveMasks_t<TMask>::BISHOP_OCCUPANCY_BITS[];

template<typename TMask>
constexpr uint64_t MoveMasks_t<TMask>::BISHOP_OCCUPANCY_MAGIC_MULTIPLIERS[];

template<typename TMask>
TMask MoveMasks_t<TMask>::BISHOP_MOVES[][4096];

template<typename TMask>
MoveMasks_t<TMask> MoveMasks_t<TMask>::sStaticInit;

typedef MoveMasks_t<Mask> MoveMasks;

}
