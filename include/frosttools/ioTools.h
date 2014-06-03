#ifndef _IO_TOOLS_H
#define _IO_TOOLS_H_

#include "frosttools/autoptr.hpp"
#include <exception>
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <stdlib.h>
#include <cstring>
#define IOTOOLS_IMPL_INLINE

#ifdef IOTOOLS_IMPL_INLINE
#define IOTOOLS_FN_IMPL inline
#else
#define IOTOOLS_FN_IMPL
#endif

namespace frosttools
{
namespace IO
{
	/// Exception for IO operations
	class _XEof : public std::exception
	{
	public:
		/// Constructor
		_XEof(){}
	};

	/// Storage buffer for memory IO
	class DataBuffer: public Referenced
	{
	public:
		DataBuffer()
		{
		    bufferSize = 0;
		    buffer = NULL;
		    bLocked = 0;
		}
		~DataBuffer()
		{
			if(buffer)
			{
				delete[] buffer;
				buffer = 0;
				bufferSize = 0;
			}
		}
		/// returns allocatd data size
		size_t size()const
		{
			return bufferSize;
		}

		/// returns raw pointer to allocated data
		const char * data() const
		{
			return buffer;
		}
		/// calculate power of 2 size
		static size_t calcSize(size_t newsize)
		{
			size_t result = 1;
			while(result < newsize)
				result*=2;
			return result;
		}
		/// resize data buffer
		void resize(size_t newsize = minSize)
		{
			newsize = calcSize(newsize);
			if(bLocked)_xlocked();
			if((buffer = reinterpret_cast<char *>(buffer?realloc(buffer,newsize):malloc(newsize)))==NULL)
				_xmem();
			bufferSize = newsize;
		}

		/// increase buffer size by specified amount
		void grow(size_t amount)
		{
			resize(bufferSize + amount);
		}

		/// write bytes to buffer
		size_t write(size_t position, const void *data, size_t size)
		{
			assert(position + size <= bufferSize);
			::memcpy(buffer + position, data, size);
			return position + size;
		}

		/// read bytes from buffer
		size_t read(void * data, size_t position, size_t size) const
		{
			assert(position + size <= bufferSize);
			::memcpy(data, buffer + position, size);
			return position + size;
		}
	protected:
		char * buffer;		///< allocated data buffer
		size_t bufferSize;	///< allocated buffer size
		size_t bLocked;		///< if write operations are allowed

		enum
		{
			minSize=256
		};
		/// throw memory exception
		void _xmem()
		{
			//throw(std::exception("memory error"));
		}
		/// throw lock exception
		void _xlocked()
		{
			//throw(std::exception("buffer is locked"));
		}
	};

	typedef SharedPtr<DataBuffer> BufferPtr;

	/// Base class for memory stream
	class Stream
	{
	public:
		/// Constructor
		Stream(BufferPtr buffer)
			:buffer(buffer),current(0)
		{}

		/// Check if end of file is reached
		bool eof()
		{
			return current == buffer->size();
		}

		/// returns current position
		size_t position()const
		{
			return current;
		}

		/// set current position
		void position(size_t p)
		{
			assert(p <= buffer->size());
			current = p;
		}

		/// reset current position
		void rewind()
		{
			current = 0;
		}
		/// get data buffer pointer
		const char * data() const
		{
			return buffer->data();
		}
		/// get current size
		int size()const
		{
			return current;
		}
		/// free bytes left
		size_t left()const
		{
			assert(buffer->size() >= current);
			return buffer->size() - current;
		}
	protected:
		BufferPtr buffer;	///< Pointer to buffer object
		size_t current;		///< current buffer position
	};

	/// Input stream
	class StreamIn : public Stream
	{
	public:
		/// Constructor
		StreamIn(BufferPtr buffer) : Stream(buffer) {}

		/// read data from the stream
		int read(void *data, size_t size)
		{
			if(current + size > buffer->size())
				throw(_XEof());
			current = buffer->read(data, current, size);
			return size;
		}

		/// get data pointer
		const void * map(size_t size)
		{
			if(current + size > buffer->size())
				throw(_XEof());
			const void * result = buffer->data() + current;
			current += size;
			return result;
		}

		/// get buffer pointer
		template<class Type> const Type * map()
		{
			return (Type*)map(sizeof(Type));
		}

		/// read generic simple type from the stream
		template<class Type> int read(Type &t)	// returns position
		{
			return read(&t,sizeof(Type));
		}


		/// read string from the stream
		int read(std::string & str)
		{
			size_t size = 0;
			size_t bytes = read(size);
			if(size)
			{
				char * buffer = new char[size];
				bytes += read(buffer,size);
				str = std::string(buffer,buffer+size);
				delete []buffer;
			}
			else
			{
				str = "";
			}
			return bytes;
		}

		/// read vector from the stream
		template<class Type> int read(std::vector<Type> & t)
		{
			size_t size = 0;
			size_t bytes = read(size);
			t.resize(size);
			for(size_t i = 0; i < size; i++)
				bytes += read(t[i]);
			return bytes;
		}
	protected:
	};

	/// Output stream
	class StreamOut : public Stream
	{
	public:
		/// Constructor
		StreamOut(BufferPtr buffer) : Stream(buffer) {}

		/// Write data to buffer
		/// returns bytes written
		int write(const void *data, size_t size)
		{
			if(current + size > buffer->size() )
				buffer->grow(size);

			current = buffer->write(current, data, size);
			return size;
		}

		/// Write data to the buffer
		/// returns bytes written
		template<class Type> int write(const Type &t)	// returns position
		{
			return write(&t,sizeof(Type));
		}
		/// Write string to the buffer
		int write(const std::string & str)
		{
			size_t size = str.size();
			size_t bytes = write(size);
			bytes += write(str.c_str(),size);
			return bytes;
		}
		/// Write vector to the buffer
		template<class Type> int write(const std::vector<Type> & t)
		{
			size_t size = t.size();
			size_t bytes = write(size);
			for(size_t i = 0; i < size; i++)
				bytes += write(t[i]);
			return bytes;
		}
	};

#ifdef IOTOOLS_IMPL_INLINE
	/// read boolean from the stream
	IOTOOLS_FN_IMPL bool readBool(StreamIn & stream)
	{
		bool result;
		stream.read(result);
		return result;
	}

	/// read int32 from the stream
	IOTOOLS_FN_IMPL int readInt(StreamIn & stream)
	{
		int result;
		stream.read(result);
		return result;
	}

	/// read float32 from the stream
	IOTOOLS_FN_IMPL float readFloat(StreamIn & stream)
	{
		float result;
		stream.read(result);
		return result;
	}

	/// read string from the stream
	IOTOOLS_FN_IMPL std::string readString(StreamIn & stream)
	{
		std::string result;
		stream.read(result);
		return result;
	}

	/// write boolean to the stream
	IOTOOLS_FN_IMPL void writeBool(StreamOut & stream, bool value)
	{
		stream.write(value);
	}

	/// write int32 to the stream
	IOTOOLS_FN_IMPL void writeInt(StreamOut & stream, int value)
	{
		stream.write(value);
	}

	/// write float32 to the stream
	IOTOOLS_FN_IMPL void writeFloat(StreamOut & stream, float value)
	{
		stream.write(value);
	}

	/// write string to the stream
	IOTOOLS_FN_IMPL void writeString(StreamOut & stream, const std::string& value)
	{
		stream.write(value);
	}
#else
	IOTOOLS_FN_IMPL bool readBool(StreamIn & stream);
	IOTOOLS_FN_IMPL int readInt(StreamIn & stream);
	IOTOOLS_FN_IMPL float readFloat(StreamIn & stream);
	IOTOOLS_FN_IMPL std::string readString(StreamIn & stream);

	IOTOOLS_FN_IMPL void writeBool(StreamOut & stream);
	IOTOOLS_FN_IMPL void writeInt(StreamOut & stream);
	IOTOOLS_FN_IMPL void writeFloat(StreamOut & stream);
	IOTOOLS_FN_IMPL void writeString(StreamOut & stream);
#endif
}	// namespace IO
}	// namespace frostools
#endif
