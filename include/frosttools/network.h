#ifndef _TIGA_NETWORK_H_
#define _TIGA_NETWORK_H_

#ifndef _MSC_VER
#include <netinet/in.h>
#include <arpa/inet.h>

const int INVALID_SOCKET=-1;
#else
#include <WinSock2.h>
#include <ws2tcpip.h>
#endif

#include <string>
#include <list>

#include <exception>

#include "threads.hpp"
#include "logger.hpp"

namespace frosttools
{
/// Wraps some generic socket methods
/// For winsock2 inits socket library
/// Contains some error handling
class Network
{
    #ifdef _MSC_VER
	WSADATA wsaData;
	struct addrinfo *result, hints;
	#endif
	Log * log;
	LogNull defaultLog;
public:
	/// Defines socket type
	typedef int SOCKET;
	/// Constructor
	Network(Log * log);

	/// Socket types
	enum SocketType
	{
		SocketTCP,
		SocketUDP,
	};

	/// Creates a socket
	SOCKET createSocket(SocketType socketType);

	/// Set blocking mode for a socket
	int setBlocking(SOCKET s, bool blocks);

	~Network();

	/// Get logger
	Log * getLog() { return log;};

	/// Close socket
	static int closeSocket(SOCKET socket);
public:
	/// Common error codes
	enum ErrorType
	{
		Unknown,
		WouldBlock,
		SocketCreation,
		ConnectionClosed,
		InvalidSlot,
		InvalidArgument,
		Memory,
		BufferOverrun,
	};

	/// Get last error code
	static ErrorType getLastError();

	/// Network exception
	class Exception : public std::exception
	{
	public:
		/// Last error type
		ErrorType error;
		/// Constructor
		Exception(ErrorType error):error(error){}
		/// Get error message
		const char * what() const throw();
	};

	/// Raise error
	static void callError(ErrorType error);
};

/// Slot update mode
enum SlotUpdateMode
{
	ModeManual,
	ModeAsyncSimple,
	ModeAsyncSelect,
};

/// Init address
int mkaddr(void *addr, int *addrlen, const char *str_addr, const char *protocol);
/// Multiclient TCP network peer
class Peer
{
protected:
	typedef frosttools::threading::Mutex Mutex;				///< Defines mutex type
	typedef frosttools::threading::ScopedLock<Mutex> Lock;	///< Defines locker type
	Mutex baseLock;					///<Lock for internal synchronization
	bool reconnect;			///< Reconnect mode
	char * buffer;			///< Intermediate buffer
	size_t bufferLength;	///< Intermediate buffer length
	Network &network;		///< Reference to network helper
	bool dying;				///< If peer is closing
public:
	/// Set buffer size
	void setBufferLength(int newLength)
	{
		if(buffer)
		{
			delete[] buffer;
			buffer = NULL;
			bufferLength = 0;
		}
		buffer = new char[newLength];
		if(buffer)
		{
			bufferLength = newLength;
		}
	}

	/// Callback class for network slot events
	class Callback
	{
	public:
        friend class Peer;
        /// Destructor
		virtual ~Callback(){}
		/// return true to accept connection
		virtual bool onIncomingConnection(Peer * peer,  size_t listener, size_t listenerId) = 0;
		/// Called when slot is closed
		virtual void onClosed(Peer * peer, size_t clientId) = 0;
		/// Called when someone is connected using listening slot
		virtual void onConnected(Peer * connection, size_t clientId ) = 0;
		/// Called when slot received some data
		virtual int onReceive(Peer * connection, size_t clientId, const void * data, size_t size) = 0;
		//virtual void onReceive(Peer * connection, size_t clientId, const MessageHeader & header, IO::StreamIn & stream) = 0;
	};
	/// Callback for slot events
	Callback * listener;
	/// Slot state
	enum SlotState
	{
		Invalid = -1,
		Empty,
		Accepting,	// waiting for connection
		Connecting,	// connecting to host
		Working,
		Dying,
	};

	/// Constructor
	///  You should ensure that Network object is valid for all peer lifetime
	Peer(Network &net, int bufferSize);
	virtual ~Peer();
	/// try to send, sync call
	virtual int send(size_t peerId, const void * data, size_t size);
	/// receive data
	virtual int recv(size_t peerId, void * buffer, size_t maxLength);	/// read data from slot manually
	/// close all slots
	virtual void close();
	/// Define result type
	typedef bool Result;
	/// Assign new slot for listening. Returns slot index
	size_t listen( int port );
	/// Connect to other endpoint
	size_t connect( const char * address, int port );
	/// Disconnect specified slot
	void disconnect( size_t slotId);
	/// Get clients count
	size_t getClientsUsed() const;
	/// Update single slot
	void updateSlot(size_t slotId);
	/// Change slot mode
	void setSlotMode(size_t slotId, SlotUpdateMode mode);
	/// Check all sockets for incoming data or connectios
	void update(timeval &timeout);
	/// Send data without any delay. Can fail sending
	void setNoDelay(size_t slotId, bool value);
	/// Get number of stored bytes
	int getDataPending(size_t slotId) const;
	/// Get slot state
	SlotState getSlotState(size_t slotId) const;

protected:
	enum { maxClients = 8 };

	struct Socket;
	/// Preallocated sockets array
	Socket * sockets;
	/// Size of sockets array
	size_t socketsAllocated;
private:
	/// insert new socket
	size_t addSocket( const Socket & socket);
	/// check socket for recieving/connecting/accepting
	bool processAccepting(size_t slot);
	void processConnecting(size_t slot);
	void processRecieving(size_t slot);
};
}
#endif
