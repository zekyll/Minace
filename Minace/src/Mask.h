#pragma once

#include "Sqr.h"
#include "Intrinsics.h"
#include <cstdint>
#include <utility>
#include <iosfwd>

namespace mnc {

template<typename T>
class Mask_t
{
public:

	class MaskIterator
	{
	private:
		T mMask;

	public:

		bool operator!=(const MaskIterator& rhs) const
		{
			return mMask != rhs.mMask;
		}

		Sqr operator*() const
		{
			return Sqr(countTrailingZeros(mMask));
		}

		MaskIterator operator++()
		{
			mMask -= lowestOneBit(mMask);
			return *this;
		}
	private:

		MaskIterator(T mask)
		: mMask(mask)
		{
		}

		friend class Mask_t;
	};

private:
	T mValue;

public:

	constexpr Mask_t()
	: mValue(0ULL)
	{
	}

	constexpr Mask_t(Sqr sqr)
	: mValue(1ULL << sqr)
	{
	}

	Mask_t(std::initializer_list<T> sqrs)
	: mValue(0ULL)
	{
		for (T sqr : sqrs)
			mValue |= 1ULL << sqr;
	}

	constexpr Mask_t(T value)
	: mValue(value)
	{
	}

	constexpr bool operator==(const Mask_t& rhs) const
	{
		return mValue == rhs.mValue;
	}

	constexpr bool operator!=(const Mask_t& rhs) const
	{
		return mValue != rhs.mValue;
	}

	constexpr explicit operator bool() const
	{
		return mValue;
	}

	constexpr Mask_t operator&(const Mask_t& rhs) const
	{
		return mValue & rhs.mValue;
	}

	constexpr Mask_t operator|(const Mask_t& rhs) const
	{
		return mValue | rhs.mValue;
	}

	Mask_t& operator&=(const Mask_t& rhs)
	{
		mValue &= rhs.mValue;
		return *this;
	}

	Mask_t& operator|=(const Mask_t& rhs)
	{
		mValue |= rhs.mValue;
		return *this;
	}

	constexpr Mask_t operator~() const
	{
		return ~mValue;
	}

	MaskIterator begin() const
	{
		return MaskIterator(mValue);
	}

	MaskIterator end() const
	{
		return MaskIterator(0);
	}

	explicit constexpr operator T() const
	{
		return mValue;
	}

	std::string toStr() const
	{
		std::string s = "";
		for (Sqr sqr : * this)
			s += sqr.toStr() + " ";
		return s.empty() ? "" : s.substr(0, s.length() - 1);
	}

	friend std::ostream& operator<<(std::ostream& os, const Mask_t& mask)
	{
		return os << mask.toStr();
	}
};

typedef Mask_t<uint64_t> Mask;

}
