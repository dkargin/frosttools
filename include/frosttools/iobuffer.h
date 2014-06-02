#ifndef _IO_BUFFER_H_
#define _IO_BUFFER_H_
#pragma once

namespace frostoolls
{

/// Contains several helpers for safe IO operations.
/// Seems to be deprecated

/**
	exception for buffer reading process. is throwed in xread/xwrite functions
**/
struct XRead
{
	int read;	// bytes read
	int asked;	// bytes asked
	XRead(int r,int a):read(r),asked(a){}
};

/** 
	reads <size> bytes from stream. 
	Also check the number of bytes read with the number of bytes asked, throwing exception
**/
///////////////////////////////////////////////////////////////////////////////////
// streambuf io
///////////////////////////////////////////////////////////////////////////////////
inline bool xread(std::streambuf &stream,void *data,int size)
{
	int count=stream.sgetn((char*)data,size);
	if(count!=size)
		throw(XRead(count,size));
	return count==size;
}
inline bool xwrite(std::streambuf &stream,const void *data,int size)
{
	stream.sputn((const char*)data,size);
	return true;
}
inline bool xeof(std::streambuf &stream)
{
	int s=stream.in_avail();
	return s==0;
}
////////////////////////////////////////////////////////////////////////////////////
// stream io
////////////////////////////////////////////////////////////////////////////////////
inline bool xread(std::istream &stream,void *data,int size)
{
	int count=stream.readsome((char*)data,size);
	//int count=stream.sgetn((char*)data,size);
	if(count!=size)
		throw(XRead(count,size));
	return count==size;
}
inline bool xwrite(std::ostream &stream,const void *data,int size)
{
	stream.write((const char*)data,size);
	return true;
}
inline bool xeof(std::ios &stream)
{
	return stream.eof();
}
//////////////////////////////////////////////////////////////////////////////////
// Buffer IO
//////////////////////////////////////////////////////////////////////////////////
class IOBuffer
{	
	char * buffer;
	int bufferSize;
	typedef char * BufferPtr;
	BufferPtr begin,end;	// cover actual data.
	BufferPtr current;		// locked inside (begin,end)
	bool bRead,bLocked;
public:
	enum
	{
		minSize=256
	};
	typedef int position;
	IOBuffer(bool in,int max=minSize)
		:bRead(in),bLocked(false),current(0),begin(0),buffer(NULL),bufferSize(0)
	{
		if(!bRead)resize(max);
	}
	void rewind()
	{
		current=begin;
	}
	int size()const
	{
		return current-begin;
	}
	int left()const
	{
		return end-current;
	}
	int write(const void *data,int size)
	{
		if(!out())_xwrite();
		if(current+size>buffer+bufferSize)
			resize(bufferSize+size);

		memcpy(current,data,size);
		if(!_CrtCheckMemory( ))_xmem();
		current+=size;
		return size;
	}
	int write(const IOBuffer &buffer)
	{
		return write(buffer.begin,buffer.size());
	}
	bool eof()
	{
		return end==current;
	}	
	int read(void *data,int size)
	{
		if(!in())_xread();
		if(current+size<=end)
		{
			memcpy(data,current,size);
			current+=size;
			return size;
		}
		else
			throw(std::exception("IOBuffer eof"));
	}
	template<class Type> Type& getBefore(int position)
	{
		if(begin+position>end)
			throw(std::exception("IOBuffer eof"));
		return *reinterpret_cast<Type*>(begin+position-sizeof(Type));
	}
	template<class Type> Type& getAfter(int position)
	{
		if(begin+position+sizeof(Type)>end)
			throw(std::exception("IOBuffer eof"));
		return *reinterpret_cast<Type*>(begin+position);
	}
	position pos()const
	{
		return (position)(current-begin);
	}
	void pos(position p)
	{
		if(p>=0 && p<=end-begin)
			current=begin+p;
		else
			_xlen(p,0);

	}
	template<class Type> int write(const Type &t)	// returns position
	{
		write(&t,sizeof(Type));
		return current-sizeof(Type)-begin;
	}
	template<class Type> int read(Type &t)
	{
		int count=read(&t,sizeof(Type));
		if(count!=sizeof(Type))
			throw(XRead(count,sizeof(Type)));
		return count;
	}
	bool flip()
	{
		if(!bLocked)
		{
			if(bRead)
				end=begin+bufferSize;
			else
				end=current;
			current=begin;
			bRead=!bRead;
		}
		return bRead;
	}
	bool in()const
	{
		return bRead;
	}
	bool out()const
	{
		return !bRead;
	}
	//IOBuffer(const IOBuffer &buffer)
	//	:bRead(buffer.bRead)
	//	,data(buffer.data)
	//	,begin(buffer.begin)
	//	,end(buffer.end)
	//	,current(buffer.current)
	//{}
	
	IOBuffer readBuffer(int length)
	{
		if(current>=end || current+length>end)_xlen(current-begin,length);
		if(!in())_xread();	// only for read only buffers
		IOBuffer result(*this,current-begin,length);
		current+=length;
		return result;
	}
	//IOBuffer(const IOBuffer &buffer)
	//	:buffer(NULL),bLocked(false),begin(buffer.begin),end(buffer.end),current(buffer.current)
	//{
	//	assert(buffer.bLocked);
	//}
	~IOBuffer()
	{
		if(buffer && !bLocked)
		{
			free(buffer);
			buffer=NULL;
		}
	}
protected:
	// construct buffer pointing to region of other buffer
	IOBuffer(IOBuffer &buffer,int start,int length)
		:bRead(true),bLocked(true),buffer(NULL),bufferSize(NULL)
	{
		begin=buffer.begin+start;
		current=begin;
		end=begin+length;
	}
	int calcSize(int newsize)
	{
		int result=1;
		while(result<newsize)
			result*=2;
		return result;
	}
	// resize data buffer, and init pointers
	void resize(int newsize=minSize)
	{
		newsize=calcSize(newsize);
		if(in())_xread();
		if(bLocked)_xlocked();
		int offs=current-begin;		
		if((buffer=reinterpret_cast<char *>(buffer?realloc(buffer,newsize):malloc(newsize)))==NULL)_xmem();	// throw exception
		bufferSize=newsize;
		
		begin=reinterpret_cast<char *>(buffer);
		end=begin+bufferSize;
		current=begin+offs;
	}
	void _xmem()
	{
		throw(std::exception("memory error"));
	}
	void _xlocked()
	{
		throw(std::exception("buffer is locked"));
	}
	void _xlen(int pos,int len)
	{
		throw(std::exception("buffer overflow"));
	}
	void _xread()
	{
		throw(std::exception("read only"));
	}
	void _xwrite()
	{
		throw(std::exception("write only"));
	}
};
//
//inline bool xread(IOBuffer &stream,void *data,int size)
//{
//	int count=stream.read(data,size);
//	if(count!=size)
//		throw(XRead(count,size));
//	return false;
//}
//inline bool xwrite(IOBuffer &stream,const void *data,int size)
//{
//	int count=stream.write(data,size);
//	return count==size;
//}
//inline bool xeof(IOBuffer &stream)
//{
//	return stream.eof();
//}
//////////////////////////////////////////////////////////////////////////////////
// generic IO
//////////////////////////////////////////////////////////////////////////////////
//template<class Stream,class Type> inline bool xread(Stream &stream,Type &t)
//{
//	return xread(stream,&t,sizeof(Type));
//}
//template<class Stream,class Type> inline bool xwrite(Stream &stream,const Type &t)
//{
//	return xwrite(stream,&t,sizeof(Type));
//}

// get from stream buffer(size) and put it into stringstream
//inline std::stringbuf getBuffer(std::streambuf &stream,int size)
//{
//	std::vector<char> data(size);
//	if(size)
//		xread(stream,&data.front(),size);	
//	return std::stringbuf(size?std::string(&data.front(),size):"");
//}
//template<class Stream> inline std::streambuf & operator <<(Stream &stream,const Buffer &buffer)
//{
//	xwrite(stream,buffer.data,buffer.length);
//	return stream;
//}
//inline std::streambuf & operator <<(std::streambuf &stream,const std::stringbuf &buffer)
//{
//	//buffer.rdbuf
//	std::vector<char> data(buffer.str().size());
//	
//	int size=buffer.str().size();
//	if(size)
//	{
//		memcpy(&data.front(),buffer.str().c_str(),size);
//		xwrite(stream,&data.front(),size);
//	}
//	return stream;
//}

//template<class Stream,class Real,int D> inline Stream & operator<<(Stream &stream,const Vector<Real,D> &vec)
//{
//	xwrite(stream,vec);
//	return stream;
//}
//template<class Stream,class Real,int D> inline Stream & operator>>(Stream &stream,Vector<Real,D> &vec)
//{
//	xread(stream,vec);
//	return stream;
//}

typedef IOBuffer MsgBuffer;
typedef IOBuffer MsgBufferImpl;

} // namespace frosttools
#endif
