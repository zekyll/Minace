#pragma once

#include "StateInfo.h"
#include <memory>
#include <stdexcept>
#include <vector>

#define CM_HASHINFO 0

namespace cm {

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

template<typename TValue>
class TranspositionTable
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

	size_t mCapacity;

	size_t mMask;

	std::vector<Entry> entries;

	unsigned mHits, mLookups;

public:

	TranspositionTable(size_t initialCapacity = DEFAULT_INITIAL_CAPACITY)
	: mRemovedCount(0), mHits(0), mLookups(0)
	{
		clear(initialCapacity);
	}

	void put(const TValue& value)
	{
		if (mReservedCount >= mCapacity >> 1)
			rehash();

		size_t h = (size_t) mGedId(value) & mMask;
		size_t d = 1;
		while (entries[h].flag != EMPTY && entries[h].flag != REMOVED &&
				mGedId(entries[h].value) != mGedId(value))
			h = (h + d++) & mMask;
		if (entries[h].flag != USED)
			++mReservedCount;

		entries[h].flag = 1;
		entries[h].value = value;
	}

	const TValue* get(uint64_t id)
	{
#ifdef CM_HASHINFO
		++mLookups;
#endif

		size_t h = (size_t) id & mMask;
		size_t d = 1;
		while (entries[h].flag != EMPTY) {
			if (entries[h].flag != REMOVED && id == mGedId(entries[h].value)) {
#ifdef CM_HASHINFO
				++mHits;
#endif
				return &entries[h].value;
			}
			h = (h + d++) & mMask;
		}

		return nullptr;
	}

	void clear(size_t initialCapacity = DEFAULT_INITIAL_CAPACITY)
	{
		mCapacity = initialCapacity;
		if (mCapacity < 8)
			throw std::invalid_argument("Initial capacity too small.");
		mMask = mCapacity - 1;
		entries = std::vector<Entry > (mCapacity);
		mReservedCount = 0;
		mRemovedCount = 0;
	}

	size_t size()
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

	size_t capacity()
	{
		return mCapacity;
	}

	uint64_t hits()
	{
		return mHits;
	}

	uint64_t lookups()
	{
		return mLookups;
	}

private:

	void rehash()
	{
		size_t newCapacity = mCapacity;

		if (mReservedCount - mRemovedCount >= mCapacity >> 1)
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
