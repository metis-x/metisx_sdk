#pragma once

struct ThreadAllocator
{
	static constexpr uint64_t ALLOCATION_FAIL = 64ull;

	ThreadAllocator()	{}

	inline void init(uint64_t numMu)
	{
		uint64_t numThread = numMu * NUM_THREAD_PER_MU;
		_bitmap = (1ull << numThread) - 1;
	}
	
	inline uint64_t allocById(uint64_t id)
	{
		if ((_bitmap & BITMAP_FROM_BITPOS(id)) == 0)
		{
			return ALLOCATION_FAIL;
		}

		_bitmap &= ~(BITMAP_FROM_BITPOS(id)); 
		return id;
	}

	inline uint64_t alloc() 
	{
		uint64_t bitpos;
		__countTailingZero(_bitmap, bitpos);
		if (bitpos != ALLOCATION_FAIL)
		{
			_bitmap &= ~(1ull << bitpos);
			return bitpos;
		}

		return ALLOCATION_FAIL;
	}

	inline void updateThreadBitmap(uint64_t newThreadBitmap)
	{
		_bitmap = newThreadBitmap;
	}

	inline void free(uint64_t id)
	{
		_bitmap |= (1ull << id);
	}
	
	inline uint64_t isAvail(void)
	{
		uint64_t result = true;
		if (_bitmap == 0)
		{
			result = false;
		}

		return result;
	}

	uint64_t _bitmap;
};
