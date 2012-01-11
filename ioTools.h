#ifndef _IO_TOOLS_H
#define _IO_TOOLS_H_

#include "autoptr.hpp"

#define IOTOOLS_IMPL_INLINE

#ifdef IOTOOLS_IMPL_INLINE
#define IOTOOLS_FN_IMPL inline
#else
#define IOTOOLS_FN_IMPL
#endif

namespace IO
{	
	class _XEof : public std::exception
	{
	public:
		_XEof(){}
		virtual const char * what() const
		{
			return "end of file reached";
		}
	};

	class DataBuffer: public Referenced
	{		
	public:
		DataBuffer():bufferSize(0),buffer(NULL),bLocked(0){}
		~DataBuffer()
		{
			if(buffer)
			{
				delete[] buffer;
				buffer = 0;
				bufferSize = 0;
			}
		}
		size_t size()const
		{
			return bufferSize;
		}

		const char * data() const
		{
			return buffer;
		}
		// calculate power of 2 size
		static size_t calcSize(size_t newsize)
		{
			size_t result = 1;
			while(result < newsize)
				result*=2;
			return result;
		}
		// resize data buffer
		void resize(size_t newsize = minSize)
		{
			newsize = calcSize(newsize);
			if(bLocked)_xlocked();	
			if((buffer = reinterpret_cast<char *>(buffer?realloc(buffer,newsize):malloc(newsize)))==NULL)
				_xmem();
			bufferSize = newsize;
		}
		void grow(size_t amount)
		{
			resize(bufferSize + amount);
		}

		size_t write(size_t position, const void *data, size_t size)
		{
			assert(position + size <= bufferSize);
			::memcpy(buffer + position, data, size);
			return position + size;
		}

		size_t read(void * data, size_t position, size_t size) const
		{
			assert(position + size <= bufferSize);
			::memcpy(data, buffer + position, size);
			return position + size;
		}
	protected:
		char * buffer;
		size_t bufferSize;
		size_t bLocked;
		enum
		{
			minSize=256
		};
		void _xmem()
		{
			throw(std::exception("memory error"));
		}
		void _xlocked()
		{
			throw(std::exception("buffer is locked"));
		}
	};
	typedef std::shared_ptr<DataBuffer> BufferPtr;

	class Stream
	{
	public:
		Stream(BufferPtr buffer)
			:buffer(buffer),current(0)
		{}

		bool eof()
		{
			return current == buffer->size();
		}
		
		size_t position()const
		{
			return current;
		}

		void position(size_t p)
		{
			assert(p <= buffer->size());
			current = p;
		}

		void rewind()
		{
			current = 0;
		}

		int size()const
		{
			return current;
		}
		// free bytes left
		size_t left()const
		{
			assert(buffer->size() >= current);
			return buffer->size() - current;
		}
	protected:
		BufferPtr buffer;		
		size_t current;		// current position
	};

	class StreamIn : public Stream
	{		
	public:
		StreamIn(BufferPtr buffer) : Stream(buffer) {}		

		// return 
		int read(void *data, size_t size)
		{	
			if(current + size > buffer->size())
				throw(_XEof());
			current = buffer->read(data, current, size);			
			return size;					
		}

		const void * map(size_t size)
		{
			if(current + size > buffer->size())
				throw(_XEof());
			const void * result = buffer->data() + current;
			current += size;
			return result;
		}

		template<class Type> const Type * map()
		{
			return (Type*)map(sizeof(Type));
		}

		template<class Type> int read(Type &t)	// returns position
		{
			return read(&t,sizeof(Type));
		}

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

	class StreamOut : public Stream
	{		
	public:
		StreamOut(BufferPtr buffer) : Stream(buffer) {}		

		// return bytes written
		int write(const void *data, size_t size)
		{
			if(current + size > buffer->size() )
				buffer->grow(size);

			current = buffer->write(current, data, size);			
			return size;
		}		

		template<class Type> int write(const Type &t)	// returns position
		{
			return write(&t,sizeof(Type));			
		}	
		int write(const std::string & str)
		{
			size_t size = str.size();
			size_t bytes = write(size);
			bytes += write(str.c_str(),size);
			return bytes;
		}
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
	IOTOOLS_FN_IMPL bool readBool(StreamIn & stream)
	{
		bool result;
		stream.read(result);
		return result;
	}

	IOTOOLS_FN_IMPL int readInt(StreamIn & stream)
	{
		int result;
		stream.read(result);
		return result;
	}

	IOTOOLS_FN_IMPL float readFloat(StreamIn & stream)
	{
		float result;
		stream.read(result);
		return result;
	}

	IOTOOLS_FN_IMPL std::string readString(StreamIn & stream)
	{
		std::string result;
		stream.read(result);
		return result;
	}

	IOTOOLS_FN_IMPL void writeBool(StreamOut & stream, bool value)
	{
		stream.write(value);
	}

	IOTOOLS_FN_IMPL void writeInt(StreamOut & stream, int value)
	{
		stream.write(value);
	}

	IOTOOLS_FN_IMPL void writeFloat(StreamOut & stream, float value)
	{
		stream.write(value);
	}

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

}
#endif