#ifndef _TIGA_NETWORK_H_
#define _TIGA_NETWORK_H_

#ifndef _MSC_VER
#include <netinet/in.h>
#include <arpa/inet.h>

const int INVALID_SOCKET=-1;
#else
#include <ws2tcpip.h>
#endif

#include <exception>

#include "threads.hpp"
#include "logger.hpp"

/// Wraps some generic socket methods
/// For winsock2 inits socket library
/// Contains some error handling
/// TODO: move all log related code to network from Peer
class Network
{
    #ifdef _MSC_VER
	WSADATA wsaData;
	struct addrinfo *result, hints;
	#endif
	Log * log;
public:
	typedef int SOCKET;
	Network(Log * log);
	enum SocketType
	{
		SocketTCP,
		SocketUDP,
	};

	SOCKET createSocket(SocketType socketType);
	int setBlocking(SOCKET s, bool blocks);

	~Network();

	Log * getLog() { return log;};

	static int closeSocket(SOCKET socket);
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
		BufferOverrun,
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

enum SlotUpdateMode
{
	ModeManual,
	ModeAsyncSimple,
	ModeAsyncSelect,
};

int mkaddr(void *addr, int *addrlen, const char *str_addr, const char *protocol);

class Peer
{
protected:
	typedef Threading::Mutex Mutex;
	typedef Threading::ScopedLock<Mutex> Lock;
	Mutex baseLock;
	bool reconnect;
	char * buffer;
	size_t bufferLength;
	Network &network;
	bool dying;
public:

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

	class Callback
	{
	public:
        friend class Peer;
		virtual ~Callback(){}
		// return true to accept connection
		virtual bool onIncomingConnection(Peer * peer,  size_t listener, size_t listenerId) = 0;
		virtual void onClosed(Peer * peer, size_t clientId) = 0;
		virtual void onConnected(Peer * connection, size_t clientId ) = 0;
		virtual int onRecieve(Peer * connection, size_t clientId, const void * data, size_t size) = 0;
		//virtual void onRecieve(Peer * connection, size_t clientId, const MessageHeader & header, IO::StreamIn & stream) = 0;
	};

	Callback * listener;

	enum SlotState
	{
		Invalid = -1,
		Empty,
		Accepting,	// waiting for connection
		Connecting,	// connecting to host
		Working,
		Dying,
	};

	Peer(Network &net, int bufferSize);
	virtual ~Peer();
	// try to send, sync call
	virtual int send(size_t peerId, const void * data, size_t size);
	// TODO: implement
	virtual int recv(size_t peerId, void * buffer, size_t maxLength);	/// read data from slot manually
	// TODO: implement
	virtual void close();

	typedef bool Result;
	size_t listen( int port );
	size_t connect( const char * address, int port );
	size_t getClientsUsed() const;
    /// enable blocking/unblocking socket handling

	void updateSlot(size_t slotId);	/// update slot manually

	void setSlotMode(size_t slotId, SlotUpdateMode mode);
	void update(timeval &timeout);

	void setNoDelay(size_t slotId, bool value);

	int getDataPending(size_t slotId) const;
	SlotState getSlotState() const;

protected:
	enum { maxClients = 8 };

	struct Socket;
	Socket * sockets;
	size_t socketsAllocated;
private:
	/// insert new socket
	size_t addSocket( const Socket & socket);
	/// check socket for recieving/connecting/accepting
	//void checkSockets(timeval &timeout);
	bool processAccepting(size_t i);
	void processConnecting(size_t i);
	void processRecieving(size_t i);
};

enum ServiceFlags
{
	ServiceFlagSameIP = 1,	/// use broadcaster's ip
};

struct ServiceDesc
{
	union
	{
		char signature[4];
		unsigned int signatureInt;
	};
	unsigned short port;
	unsigned short flags;
	char name[64];
	char address[64];
	ServiceDesc();
	ServiceDesc(const ServiceDesc &desc);
};

#include <string>
#include <list>

struct BroadcasterData
	{
	std::string broadcastAddress;
	int timeout;
	typedef std::list<ServiceDesc> Services;
	Services services;

	BroadcasterData();

	void addService(const ServiceDesc & desc);
};

void sendServiceDesc(const ServiceDesc & desc, Network::SOCKET socket, sockaddr_in & address);
void broadcastServices(BroadcasterData::Services & services, Network::SOCKET socket, sockaddr_in & address);
void run_broadcast(Network & network, BroadcasterData * br);
#endif
