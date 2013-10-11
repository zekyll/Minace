#pragma once

#include "StateInfo.h"
#include <memory>
#include <stdexcept>
#include <vector>

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

	TpHash<TValue> getid;

	size_t reservedCount = 0;

	size_t removedCount = 0;

	size_t capacity;

	size_t mask;

	std::vector<Entry> entries;

public:

	TranspositionTable(size_t initialCapacity = DEFAULT_INITIAL_CAPACITY)
	: removedCount(0)
	{
		clear(initialCapacity);
	}

	void put(const TValue& value)
	{
		if (reservedCount >= capacity >> 1)
			rehash();

		size_t h = (size_t) getid(value) & mask;
		size_t d = 1;
		while (entries[h].flag != EMPTY && entries[h].flag != REMOVED &&
				getid(entries[h].value) != getid(value))
			h = (h + d++) & mask;
		if (entries[h].flag != USED)
			++reservedCount;

		entries[h].flag = 1;
		entries[h].value = value;
	}

	const TValue* get(uint64_t id)
	{
		size_t h = (size_t) id & mask;
		size_t d = 1;
		while (entries[h].flag != EMPTY) {
			if (entries[h].flag != REMOVED && id == getid(entries[h].value))
				return &entries[h].value;
			h = (h + d++) & mask;
		}

		return nullptr;
	}

	void clear(size_t initialCapacity = DEFAULT_INITIAL_CAPACITY)
	{
		capacity = initialCapacity;
		if (capacity < 8)
			throw std::invalid_argument("Initial capacity too small.");
		mask = capacity - 1;
		entries = std::vector<Entry > (capacity);
		reservedCount = 0;
		removedCount = 0;
	}

	size_t size()
	{
		return reservedCount - removedCount;
	}

	void remove(uint64_t id)
	{
		size_t h = (int) id & mask;
		size_t d = 1;
		while (entries[h].flag != EMPTY) {
			if (id == getid(entries[h].value)) {
				entries[h].flag = REMOVED;
				++removedCount;
				return;
			}
			h = (h + d++) & mask;
		}
	}

private:

	void rehash()
	{
		size_t newCapacity = capacity;

		if (reservedCount - removedCount >= capacity >> 1)
			newCapacity *= GROWTH_FACTOR;
		size_t newMask = newCapacity - 1;
		std::vector<Entry> newEntries(newCapacity);

		copyEntries(newEntries, newMask);

		capacity = newCapacity;
		mask = newMask;
		entries = newEntries;
		reservedCount -= removedCount;
		removedCount = 0;
	}

	void copyEntries(std::vector<Entry>& newEntries, size_t newMask)
	{
		for (size_t i = 0; i < capacity; ++i) {
			if (entries[i].flag != EMPTY && entries[i].flag != REMOVED) {
				size_t h = (size_t) getid(entries[i].value) & newMask;
				size_t d = 1;
				while (newEntries[h].flag != EMPTY)
					h = (h + d++) & newMask;
				newEntries[h] = entries[i];
			}
		}
	}
};

}
