#pragma once

#include "StateInfo.h"
#include "Util.h"
#include "Config.h"
#include "Scores.h"
#include <memory>
#include <stdexcept>
#include <vector>

namespace mnc {

/* Transposition table consisting of buckets with 2 slots in each bucket. 1st slot is only replaced
 * if new entry has greater depth. Each entry has age, and entries from previous searches are
 * always replaced.
 *
 * In addition to capacity the table has a limit for the number of entries which can change
 * dynamically (MIN_LIMIT <= limit <= capacity) according to usage. This allows better data
 * locality and slightly better performance with small search depths.
 */
template<typename TValue>
class TranspositionTable
{
private:

	static constexpr size_t DEFAULT_INITIAL_CAPACITY = 16;

	static constexpr unsigned BUCKET_SIZE = 2;

	static constexpr unsigned MIN_LIMIT = 8;

	size_t mCapacity, mLimit, mMask, mLimitGrowThreshold;

	std::vector<TValue> mEntries;

	size_t mSize;

	unsigned mPrevWrites, mCurrentWrites, mTotalWrites, mLookups;

	uint8_t mSearchIdx;

public:

	explicit TranspositionTable(size_t capacityBytes)
	: mPrevWrites(0), mTotalWrites(0), mLookups(0), mSearchIdx(0)
	{
		clear(capacityBytes);
	}

	void put(const StateInfo& value)
	{
		assert(Scores::isValid(value.score));
		assert(value.bestMove || value.score == -Scores::MATE || value.score == Scores::DRAW);

		if (++mCurrentWrites > mLimitGrowThreshold)
			growLimit();

		size_t bucket = (size_t) value.id & mMask;

		if (addToBucket(bucket, value, mSearchIdx))
			++mSize;
	}

	const TValue* get(uint64_t id)
	{
#if CM_EXTRA_INFO
		++mLookups;
#endif

		size_t b = (size_t) id & mMask;
		if (mEntries[b].id == id)
			return &mEntries[b];
		if (mEntries[b + 1].id == id)
			return &mEntries[b + 1];

		return nullptr;

	}

	void clear(size_t capacityBytes)
	{
		mCapacity = roundUpToPowerOfTwo(capacityBytes / sizeof (TValue) + 1) / 2;
		if (mCapacity < MIN_LIMIT)
			throw std::invalid_argument("Capacity too small.");
		mLimit = MIN_LIMIT;
		mMask = mLimit - 1 - (BUCKET_SIZE - 1);
		mLimitGrowThreshold = mLimit == mCapacity ? (size_t) - 1 : mLimit / 2;
		mEntries = std::vector<TValue> (mCapacity);
		for (TValue& si : mEntries) {
			si.id = 0;
			si.age = 0;
		}
		mSize = 0;
		mCurrentWrites = 0;
	}

	size_t size() const
	{
		return mSize;
	}

	size_t limit() const
	{
		return mLimit;
	}

	size_t capacity() const
	{
		return mCapacity;
	}

	uint64_t lookups() const
	{
		return mLookups;
	}

	uint64_t writes() const
	{
		return mCurrentWrites + mTotalWrites;
	}

	void startNewSearch()
	{
		if (++mSearchIdx == 0)
			++mSearchIdx;

		if (std::max(mCurrentWrites, mPrevWrites) < mLimit << 2 && mLimit > MIN_LIMIT)
			shrinkLimit();

		mTotalWrites += mCurrentWrites;
		mPrevWrites = mCurrentWrites;
		mCurrentWrites = 0;
	}

private:

	bool addToBucket(size_t b, const StateInfo& value, uint8_t age)
	{
		// Replaces 1st entry if it belongs to previous search or new entry has greater depth.
		// Otherwise always replaces 2nd entry.
		if (mEntries[b].age == mSearchIdx && value.depth <= mEntries[b].depth)
			++b;
		bool isNew = !mEntries[b].age;
		mEntries[b] = value;
		mEntries[b].age = age;
		return isNew;
	}

	void growLimit()
	{
		size_t mNewLimit = 2 * mLimit;
		size_t mNewMask = mNewLimit - 1 - (BUCKET_SIZE - 1);

		for (size_t i = 0; i < mLimit; ++i) {
			if (mEntries[i].age) {
				size_t b = (size_t) mEntries[i].id & mNewMask;
				if (b >= mLimit) {
					if (!addToBucket(b, mEntries[i], mEntries[i].age))
						--mSize;
					mEntries[i].id = 0;
					mEntries[i].age = 0;
				}
			}
		}

		mLimit = mNewLimit;
		mMask = mNewMask;
		mLimitGrowThreshold = mLimit == mCapacity ? (size_t) - 1 : mLimit / 2;
	}

	void shrinkLimit()
	{
		size_t mNewLimit = mLimit / 2;
		size_t mNewMask = mNewLimit - 1 - (BUCKET_SIZE - 1);

		for (size_t i = mNewLimit; i < mLimit; ++i) {
			if (mEntries[i].age) {
				size_t b = (size_t) mEntries[i].id & mNewMask;
				if (!addToBucket(b, mEntries[i], mEntries[i].age))
					--mSize;
				mEntries[i].id = 0;
				mEntries[i].age = 0;
			}
		}

		mLimit = mNewLimit;
		mMask = mNewMask;
		mLimitGrowThreshold = mLimit / 2;
	}
};

}
