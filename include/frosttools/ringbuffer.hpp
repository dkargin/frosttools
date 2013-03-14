#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <string.h>
#include <assert.h>

class RingBuffer
{
	char * buffer;
	int maxSize;	/// allocated size
	int currentSize;
	int markerRead;		/// current read marker
	int markerWrite;		/// current write marker
public:

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

	void clear()
	{
		if(buffer)
		{
			delete[]buffer;
			maxSize = 0;
		}
	}

	int moveRead(size_t distance)
	{
		markerRead += distance;
		if(markerRead >= maxSize)
			markerRead -= maxSize;
		return markerRead;
	}

	int moveWrite(size_t distance)
	{
		markerWrite += distance;
		if(markerWrite >= maxSize)
			markerWrite -= maxSize;
	}

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

	bool overwrites() const
	{
		return true;
	}

	size_t getMaxSize() const
	{
		return maxSize;
	}

	void clean()
	{
		markerWrite = 0;
		markerRead = 0;
		currentSize = 0;
	}

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
template<class Type,int Max>
struct RingArray
{
	typedef Type value_type;
	typedef unsigned int size_type;
	static const size_type max=Max;
	value_type values[max];
	size_type count;
	size_type start;
	RingArray():count(0),start(0){}
	// add value to buffer end
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
	void push_front(const value_type &val)
	{
		start=index(-1);
		values[index(0)]=val;
		if(count<max)		
			count++;		
	}
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
	inline bool empty() const
	{
		return count==0;
	}
	inline size_type size() const
	{
		return count;
	}
	const value_type & operator[](const size_type &i) const
	{
		return values[index(i)];
	}
	value_type & operator[](const size_type &i)
	{
		return values[index(i)];
	}
	size_type index(const size_type &i) const
	{		
		return (start+i<0?labs(i+max):i)%max;
	}
};

#endif
