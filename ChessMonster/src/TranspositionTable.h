#pragma once

#include "StateInfo.h"
#include "Util.h"
#include <memory>
#include <stdexcept>
#include <vector>

#define CM_HASHINFO 0

namespace cm {

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
		if (++mCurrentWrites > mLimitGrowThreshold)
			growLimit();

		size_t bucket = (size_t) value.id & mMask;

		if (addToBucket(bucket, value, mSearchIdx))
			++mSize;
	}

	const TValue* get(uint64_t id)
	{
#if CM_HASHINFO
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


template <class T>
class TpHash;

template<>
class TpHash<StateInfo>
{
public:

	uint64_t operator()(const StateInfo& s) const
	{
		return s.id;
	}
};

template<>
class TpHash<uint64_t>
{
public:

	uint64_t operator()(uint64_t s) const
	{
		return s;
	}
};

/* A generic hash table using open addressing and quadratic probing. */
template<typename TValue>
class HashTable
{
private:

	struct Entry
	{
		unsigned flag;
		TValue value;

		Entry()
		: flag(EMPTY)
		{
		}
	};

	static constexpr size_t DEFAULT_INITIAL_CAPACITY = 16;

	static constexpr size_t GROWTH_FACTOR = 2;

	static constexpr unsigned EMPTY = 0;

	static constexpr unsigned USED = 1;

	static constexpr unsigned REMOVED = 2;

	TpHash<TValue> mGedId;

	size_t mReservedCount = 0;

	size_t mRemovedCount = 0;

	size_t mCapacity, mMaxCapacity;

	size_t mMask;

	std::vector<Entry> entries;

	unsigned mLookups;

public:

	explicit HashTable(size_t initialBytes = 512)
	: mRemovedCount(0), mLookups(0)
	{
		clear(initialBytes);
	}

	void put(const TValue& value)
	{
		size_t h = (size_t) mGedId(value) & mMask;
		size_t d = 1;
		while (entries[h].flag != EMPTY && entries[h].flag != REMOVED &&
				mGedId(entries[h].value) != mGedId(value))
			h = (h + d++) & mMask;
		if (entries[h].flag != USED)
			++mReservedCount;

		entries[h].flag = 1;
		entries[h].value = value;

		if (mReservedCount > mCapacity >> 1)
			rehash();
	}

	const TValue* get(uint64_t id)
	{
#if CM_HASHINFO
		++mLookups;
#endif

		size_t h = (size_t) id & mMask;
		size_t d = 1;
		while (entries[h].flag != EMPTY) {
			if (entries[h].flag != REMOVED && id == mGedId(entries[h].value)) {
				return &entries[h].value;
			}
			h = (h + d++) & mMask;
		}

		return nullptr;
	}

	void clear(size_t initialBytes = 512)
	{
		mCapacity = roundUpToPowerOfTwo(initialBytes / sizeof (Entry) + 1) / 2;
		if (mCapacity < 8)
			throw std::invalid_argument("Initial capacity too small.");
		mMask = mCapacity - 1;
		entries = std::vector<Entry > (mCapacity);
		mReservedCount = 0;
		mRemovedCount = 0;
	}

	size_t size() const
	{
		return mReservedCount - mRemovedCount;
	}

	void remove(uint64_t id)
	{
		size_t h = (int) id & mMask;
		size_t d = 1;
		while (entries[h].flag != EMPTY) {
			if (id == mGedId(entries[h].value)) {
				entries[h].flag = REMOVED;
				++mRemovedCount;
				return;
			}
			h = (h + d++) & mMask;
		}
	}

	size_t capacity() const
	{
		return mCapacity;
	}

	uint64_t lookups() const
	{
		return mLookups;
	}

private:

	void rehash()
	{
		size_t newCapacity = mCapacity;

		if (size() > mCapacity >> 1)
			newCapacity *= GROWTH_FACTOR;
		size_t newMask = newCapacity - 1;
		std::vector<Entry> newEntries(newCapacity);

		copyEntries(newEntries, newMask);

		mCapacity = newCapacity;
		mMask = newMask;
		entries = std::move(newEntries);
		mReservedCount -= mRemovedCount;
		mRemovedCount = 0;
	}

	void copyEntries(std::vector<Entry>& newEntries, size_t newMask)
	{
		for (size_t i = 0; i < mCapacity; ++i) {
			if (entries[i].flag != EMPTY && entries[i].flag != REMOVED) {
				size_t h = (size_t) mGedId(entries[i].value) & newMask;
				size_t d = 1;
				while (newEntries[h].flag != EMPTY)
					h = (h + d++) & newMask;
				newEntries[h] = entries[i];
			}
		}
	}
};


}
