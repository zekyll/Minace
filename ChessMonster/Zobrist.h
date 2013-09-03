#pragma once

#include "Player.h"
#include "Sqr.h"
#include "Piece.h"
#include <random>
#include <array>
#include <cstdint>

namespace cm {

template<typename T>
class Zobrist_t
{
public:
	static std::array<T, Sqr::COUNT> PIECE_SQR_RND[Player::COUNT][Piece::COUNT];

	static T PLAYER_RND;

	static std::array<T, Sqr::COUNT> EN_PASSANT_RND;

	static std::array<T, Sqr::COUNT> CASTLINGRIGHTS_RND;

	static T EMPTY_RND;

private:

	Zobrist_t()
	{
		std::mt19937_64 rng(999);
		std::uniform_int_distribution<T> rnd;
		for (auto& x : PIECE_SQR_RND) {
			for (auto& y : x) {
				for (auto& z : y)
					z = rnd(rng);
			}
		}
		PLAYER_RND = rnd(rng);
		EMPTY_RND = rnd(rng);
		for (T& x : EN_PASSANT_RND)
			x = rnd(rng);
		for (T& x : CASTLINGRIGHTS_RND)
			x = rnd(rng);

	}

	static Zobrist_t sInit;
};

template<typename T>
std::array<T, Sqr::COUNT> Zobrist_t<T>::PIECE_SQR_RND[Player::COUNT][Piece::COUNT];

template<typename T>
T Zobrist_t<T>::PLAYER_RND;

template<typename T>
std::array<T, Sqr::COUNT> Zobrist_t<T>::EN_PASSANT_RND;

template<typename T>
std::array<T, Sqr::COUNT> Zobrist_t<T>::CASTLINGRIGHTS_RND;

template<typename T>
T Zobrist_t<T>::EMPTY_RND;

template<typename T>
Zobrist_t<T> Zobrist_t<T>::sInit;

typedef Zobrist_t<uint64_t> Zobrist;

}