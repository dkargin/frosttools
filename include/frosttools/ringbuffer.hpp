#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <string.h>
#include <assert.h>


namespace frosttools
{
/// \addtogroup Containers
/// @{

/// Ring buffer
/**
 * Preallocated ring buffer for read/write operations
 * Data is overwritten when the buffer is full
 */
class RingBuffer
{
	char * buffer;		///< actual data
	int maxSize;		///< allocated size
	int currentSize;	///< current buffer sie
	int markerRead;		///< read marker
	int markerWrite;	///< write marker
public:
	/// Constructor
	RingBuffer(int max)
	{
		buffer = NULL;
		maxSize = 0;
		markerRead = 0;
		markerWrite = 0;
		currentSize = 0;
	}

	~RingBuffer()
	{
		clear();
	}

	/// remove all data
	void clear()
	{
		if(buffer)
		{
			delete[]buffer;
			maxSize = 0;
		}
	}

	/// move read marker
	int moveRead(size_t distance)
	{
		markerRead += distance;
		if(markerRead >= maxSize)
			markerRead -= maxSize;
		return markerRead;
	}

	/// move write marker
	int moveWrite(size_t distance)
	{
		markerWrite += distance;
		if(markerWrite >= maxSize)
			markerWrite -= maxSize;
	}

	/// resize buffer
	void resize(size_t newSize)
	{
		if(maxSize == newSize)
			return;
		clear();
		if(newSize == 0)
			return;

		buffer = new char[newSize];
		if(buffer)
		{
			maxSize = newSize;
			markerRead = 0;
			markerWrite = 0;
			currentSize = 0;
		}
	}

	/// check if overwriting is allowed
	bool overwrites() const
	{
		return true;
	}

	/// get max buffer size
	size_t getMaxSize() const
	{
		return maxSize;
	}

	/// clean buffer contents
	void clean()
	{
		markerWrite = 0;
		markerRead = 0;
		currentSize = 0;
	}

	/// try to get data from buffer
	const char *peek(size_t &asize)
	{
		if(asize == 0)
			asize = currentSize;
		if(asize > currentSize)
			asize = currentSize;
		if(asize + markerRead > maxSize)
			asize = (maxSize - markerRead);
		const char * result = buffer + markerRead;

		moveRead(asize);

		currentSize -= asize;
		return result;
	}
	/// Forced writing. If storage is full - overwrite old data
	int writeForce(const char * source, int size)
	{
		assert(size <= maxSize);
		/// 1. check border crossing
		if(markerWrite+size > maxSize)
		{
			int firstLength = maxSize - markerWrite;
			memcpy(buffer + markerWrite, source, firstLength);
			memcpy(buffer, source + firstLength, size - firstLength);
		}
		else
		{
			memcpy(buffer + markerWrite, source, size);
		}
		/// move marker
		moveWrite(size);
		/// increase stored size
		currentSize += size;
		if(currentSize > maxSize)
		{
			currentSize = maxSize;
			markerRead = markerWrite;
		}
		return 0;
	}

	/// get available data size
	size_t available()
	{
		return currentSize;
	}

	/// Strict data reading. Returns bytes got from buffer
	int read(char * target, int size)
	{
		assert(size <= maxSize);

		if(currentSize == 0)
			return 0;
		// trim size to available
		if(currentSize <= size)
			size = currentSize;
		/// 1. check border crossing
		if(markerRead+size > maxSize)
		{
			int firstLength = maxSize - markerRead;
			memcpy(target, buffer + markerRead, firstLength);
			memcpy(target+firstLength, buffer, size - firstLength);
		}
		else
		{
			memcpy(target, buffer + markerRead, size);
		}

		/// move marker
		markerRead += size;
		if(markerRead >= maxSize)
			markerRead -= maxSize;

		/// increase stored size
		currentSize -= size;
		return size;
	}
};

/// Ring Array
template<class Type,int Max>
struct RingArray
{
	typedef Type value_type;			///< Defines value type
	typedef unsigned int size_type;	///< Defines index type
	static const size_type max=Max;	///< Max array capacity
	value_type values[max];				///< Data storage
	size_type count;					///< Number of stored objects
	size_type start;					///< index of the first stored object


	RingArray():count(0),start(0){}
	/// add value to buffer end
	void push_back(const value_type &val)
	{
		if(count==max)
		{
			values[start]=val;
			start=index(1);
		}
		else
		{
			values[index(count)]=val;
			count++;
		}
	}

	/// add value to buffer front
	void push_front(const value_type &val)
	{
		start=index(-1);
		values[index(0)]=val;
		if(count<max)		
			count++;		
	}
	/// pop value from buffer back
	value_type pop_back()
	{
		value_type res;
		if(count)
		{			
			count--;
			res=values[index(count)];
		}
		return res;
	}
	/// pop value from buffer front
	value_type pop_front()
	{
		value_type res;
		if(count)
		{
			start=index(1);
			count--;
			res=values[index(-1)];
		}
		return res;
	}
	/// check if ring is empty
	inline bool empty() const
	{
		return count==0;
	}
	/// get current size
	inline size_type size() const
	{
		return count;
	}
	/// get value at index
	const value_type & operator[](const size_type &i) const
	{
		return values[index(i)];
	}
	/// get value at index
	value_type & operator[](const size_type &i)
	{
		return values[index(i)];
	}
	/// find value by its reference
	size_type index(const size_type &i) const
	{		
		return (start+i<0?labs(i+max):i)%max;
	}
};

} // namespace frostools

/// @}
#endif
