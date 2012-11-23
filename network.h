#ifndef _TIGA_NETWORK_H_
#define _TIGA_NETWORK_H_

#ifndef _MSC_VER
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
const int INVALID_SOCKET=-1;
#else
#include <ws2tcpip.h>
#endif
//
//#include <process.h>
#define FrostTools_Use_Types
#define FrostTools_Use_System
#define FrostTools_Use_Threads
//#include <frosttools/frosttools.h>
#include "ioTools.h"
#include "logger.hpp"

namespace Net
{
	enum NetMsgType
	{
		TypeHeartBeat,
		TypeHello,	/// the first message sent by new client
		TypeAssignID,	/// server sends it to the new client
		TypeChat,		/// common chat
	};


}

const int DEFAULT_BUFLEN = 32;
const char DEFAULT_PORT[] = "2000";

template<class Parent> class ScopedLock;

class Lockable
{
    #ifdef _MSC_VER
	CRITICAL_SECTION cs;
	#endif
public:
	friend class ScopedLock<Lockable>;
	typedef ScopedLock<Lockable> Scoped;

#ifdef WINVER
	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}

	Lockable()
	{
		InitializeCriticalSection(&cs);
	}

	virtual ~Lockable()
	{
		DeleteCriticalSection(&cs);
	}

	bool locked() const
	{
		if(TryEnterCriticalSection(const_cast<CRITICAL_SECTION*>(&cs)))
		{
			LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&cs));
			return false;
		}
		return true;
	}
#else
    void lock()
	{
	}

	void unlock()
	{
	}

	Lockable()
	{
	}

	virtual ~Lockable()
	{
	}

	bool locked() const
	{
		return false;
	}
#endif
};

template<class Parent> class ScopedLock
{
	Parent &parent;
public:
	ScopedLock(const Parent &p):parent((Parent&)p)
	{
		parent.lock();
	}
	~ScopedLock()
	{
		parent.unlock();
	}
};

class Network
{
    #ifdef _MSC_VER
	WSADATA wsaData;
	struct addrinfo *result, hints;
	#endif
	Log * log;
public:
	Network(Log * log);
	SOCKET createSocket();
	~Network();
	Log * getLog() { return log;};

	static void closeSocket(SOCKET socket);
public:
	enum ErrorType
	{
		Unknown,
		WouldBlock,
		SocketCreation,
		ConnectionClosed,
		InvalidSlot,
		InvalidArgument,
		Memory,
	};

	static ErrorType getLastError();

	class Exception : public std::exception
	{
	public:
		ErrorType error;
		Exception(ErrorType error):error(error){}
		const char * what() const throw();
	};
	static void callError(ErrorType error);
};

class Peer
{
protected:
	typedef Lockable::Scoped Lock;
	Lockable baseLock;	// to lock access to peer data from child threads
	bool reconnect;
	bool waitPeer;
	IO::BufferPtr buffer;
	Network &network;
	bool dying;
public:
    struct MessageHeader
	{
		unsigned short type;	/// message type
		unsigned short length;	/// message length
	};

	class Callback
	{
	public:
        friend class Peer;
		// return true to accept connection
		virtual bool onIncomingConnection(Peer * peer,  size_t listener, size_t listenerId) = 0;
		virtual void onClosed(Peer * peer, size_t clientId) = 0;
		virtual void onConnected(Peer * connection, size_t clientId ) = 0;
		virtual void onRecieve(Peer * connection, size_t clientId, const void * data, size_t size, size_t pendingOffset, size_t pendingSize) = 0;
		//virtual void onRecieve(Peer * connection, size_t clientId, const MessageHeader & header, IO::StreamIn & stream) = 0;
	};

	Callback * listener;

	Peer(Network &net, int bufferSize);
	virtual ~Peer();
	// try to send, sync call
	virtual int send(size_t peerId, const void * data, size_t size);
	virtual void close();
	typedef bool Result;
	size_t listen( int port );
	size_t connect( const char * address, int port );
	size_t getClientsUsed() const;
    /// enable blocking/unblocking socket handling
	void setBlockingMode(size_t slotId, bool block);
	void requestData(size_t slotId, size_t amount);
	void update(timeval &timeout);
protected:
	enum { maxClients = 8 };

	struct Socket;
	Socket * sockets;
	size_t socketsAllocated;
private:
	/// insert new socket
	size_t addSocket( const Socket & socket);
	/// check socket for recieving/connecting/accepting
	void checkSockets(timeval &timeout);
	bool processAccepting(size_t i);
	void processConnecting(size_t i);
	void processRecieving(size_t i);
};



class TextCallback : public Peer::Callback
{
public:
	virtual bool onIncomingConnection(Peer * peer,  size_t listener, size_t connectionId)
	{
		printf("Callback::onIncomingConnection(%d,%d)\n", listener, connectionId);
		return true;	// accept all by default
	}
	virtual void onClosed(Peer * peer, size_t clientId)
	{
		printf("Callback::onClosed(peer, %d)\n", clientId);
	}
	virtual void onConnected(Peer * connection, size_t clientId )
	{
		printf("Client connected to slot %d\n", clientId);		
	}
	virtual void onRecieve(Peer * connection, size_t clientId, const Peer::MessageHeader & header, IO::StreamIn & stream)
	{
		size_t size = stream.left();
		char * data = new char[size+1];
		stream.read(data, size);
		printf("recieved message:%s fom %d\n", data,clientId);
		delete []data;
	}
};
#endif
