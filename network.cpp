//#include "stdafx.h"
#include "network.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#else
#include <errno.h>
#include <fcntl.h>
#endif

const char * Network::Exception::what() const throw()
{
	switch(error)
	{
	case SocketCreation:
		return "Cannot create new socket";
	case ConnectionClosed:
		return "Connection was closed";
	case InvalidSlot:
		return "Invalid slot";
	case Memory:
		return "Memory allocation error";
	}
	return "Unknown error";
}
//typedef unsigned int (__stdcall *ThreadFunction)(void *);
void ReportErrorWin32(int errorCode, const char *whichFunc)
{
	char errorMsg[500];          // Declare a buffer to hold the generated messages
	memset(errorMsg, sizeof(errorMsg),0);            // Automatically NULL-terminate the strings
#ifdef _MSC_VER
	switch(errorCode)
	{
	case 10014:
	sprintf(errorMsg, "Bad address.");//
	break;
	case 10022:
		sprintf(errorMsg, "Invalid argument.");//
		break;
	case 10036:
		sprintf(errorMsg, "Operation now in progress.");//
		break;
	case 10037:
		sprintf(errorMsg, "Operation already in progress. ");//
		break;
	case 10038:
		sprintf(errorMsg, "Socket operation on non-socket.");//
		break;
	case 10040:
		sprintf(errorMsg, "Message too long.");//
		break;
	case 10048:
		sprintf(errorMsg, "Address already in use.");//
		break;
	case 10049:
		sprintf(errorMsg, "Cannot assign requested address.");//
		break;
	case 10050:
		sprintf(errorMsg, "Network is down.");//
		break;
	case 10053:
		sprintf(errorMsg, "Software caused connection abort.");//
		break;
	case 10054:
		sprintf(errorMsg, "Peer reset by remote side.");//
		break;
	case 10055:
		sprintf(errorMsg, "No buffer space available.");//
		break;
	case 10057:
		sprintf(errorMsg, "Socket is not connected.");//
		break;
	case 10060:
		sprintf(errorMsg, "Peer timed out.");//
		break;
	case 10061:
		sprintf(errorMsg, "Peer refused.");//
		break;
	case 10064:
		sprintf(errorMsg, "Host is down.");//
		break;
		/*  case :
		sprintf(errorMsg, "");
		break;
		*/ default:
			//sprintf(caption, "Socket Indication");
			sprintf(errorMsg, "Call to %s returned error %d!", (char *)whichFunc, errorCode);
	}
	char caption[500];
	memset(caption, sizeof(caption),0);
	sprintf(caption, "Socket Error on function %s !", (char *)whichFunc);
	MessageBoxA(NULL, errorMsg, caption, MB_OK  |  MB_ICONERROR);// show it finally
#else
	fprintf(stderr, errorMsg);
#endif
	return;
};

bool IsInvalidSocket(SOCKET socket)
{
        return socket < 0;
}

bool SocketError(int result)
{
    return result < 0;
}

Network::Network(Log * log)
	:/*result(NULL), */log(log)
{
	LogFunction(*log);
	#ifdef _MSC_VER
	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		log->line(1, "WSAStartup failed: %d\n", iResult);
		//return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if ( iResult != 0 ) {
		log->line(1, "getaddrinfo failed: %d\n", iResult);
		//WSACleanup();
		//return 1;
	}
	#endif
}

Network::ErrorType Network::getLastError()
{
	ErrorType result = Unknown;
#ifdef _MSC_VER
	int errcode = WSAGetLastError();
	switch(errcode)
	{
	case EWOULDBLOCK:
		result = WouldBlock;
	}
#else
	int errcode = errno;
	switch(errcode)
	{
	//case EWOULDBLOCK:
	//break;
	case EINTR:
        result = Unknown;
        break;
    case EBADF:
        result = Unknown;
        break;
    case EINVAL:
        return InvalidArgument;
	case EAGAIN:
		return WouldBlock;
    break;
	}
#endif
	return result;
}

void Network::closeSocket(SOCKET socket)
{
#ifdef _MSC_VER
	closesocket(socket);
#else
    close(socket);
#endif
}

SOCKET Network::createSocket()
{
	LogFunction(*log);
	return socket(AF_INET, SOCK_STREAM, 0);
}

void Network::callError(Network::ErrorType error)
{
	throw(Exception(error));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Network::~Network()
{
	LogFunction(*log);
	#ifdef _MSC_VER
	freeaddrinfo(result);
	WSACleanup();
	#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct Peer::Socket
{
	enum State
	{
		Empty,
		Accepting,	// waiting for connection
		Connecting,	// connecting to host
		Working,
		Dying,
	}state;
	SOCKET socket;
	sockaddr_in addr;
	int bytesPending;
	int bytesRecieved;
	bool toRead;	// if socket is ready for read operations
	bool blocking;	// if socket is in nonblocking mode
	Socket() : state(Empty), socket(INVALID_SOCKET), toRead(false), blocking(true), bytesPending(0), bytesRecieved(0){}
	Socket(const Socket & s) : state(s.state), socket(s.socket), addr(s.addr), bytesRecieved(s.bytesRecieved), bytesPending(s.bytesPending) {}
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Peer::Peer(Network &net, int bufferSize)
	:buffer(new IO::DataBuffer()), listener(NULL), network(net), sockets(NULL), socketsAllocated(0)
{
	buffer->resize(bufferSize);
	dying = false;
	reconnect = true;
}

Peer::~Peer()
{
	close();
}

void Peer::close()
{
	// TODO:	close all sockets
	for( size_t i = 0; i < socketsAllocated; i++)
	{
		network.closeSocket(sockets[i].socket);
	}
	free(sockets);
	sockets = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Peer::send(size_t peerId, const void * data, size_t size)
{
	if( peerId >= socketsAllocated )
	{
		throw Network::Exception(Network::InvalidSlot);
		return -1;
	}
	Socket & s = sockets[peerId];
	if( s.state != Socket::Working )
		return 0;
    int sendFlags = 0;
    #ifdef _MSC_VER
    #else
    sendFlags |= MSG_NOSIGNAL;
    #endif

	int bytesData = 0;
	if( size > 0 )
	{
		bytesData = ::send(s.socket, (const char*)data, size, sendFlags );
		if( bytesData <= 0 )
		{
			Network::ErrorType err = Network::getLastError();
			s.state = Socket::Dying;
			if( err == Network::ConnectionClosed )
			{
				return -1;
			}
			//network.getLog()->line(0,"General net: ERROR sending to invalid connection\n");
		}
		//else
		//	network.getLog()->line(0,"Sent %d bytes of data\n", bytesData);
	}
	return bytesData;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Peer::addSocket(const Peer::Socket & s)
{
	size_t newSize = socketsAllocated + 1;

	sockets = (Socket*)realloc(sockets, newSize * sizeof(Socket));
	if( sockets == NULL)
	{
		socketsAllocated = 0;
		Network::callError(Network::Memory);
		return -1;
	}
	memcpy(sockets + socketsAllocated, &s, sizeof(Socket));
	socketsAllocated = newSize;
	return socketsAllocated - 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Peer::setBlockingMode(size_t slot, bool blocking)
{
	if( slot >= socketsAllocated )
	{
		Network::callError(Network::InvalidSlot);
		return;
	}
	if( sockets[slot].blocking != blocking)
	{
#if _MSC_VER
#else
		int flags;
		flags = fcntl(sockets[slot].socket,F_GETFL,0);
		assert(flags != -1);
		fcntl(sockets[slot].socket, F_SETFL, flags | O_NONBLOCK);
#endif
		sockets[slot].blocking = blocking;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Peer::getClientsUsed() const
{
	Lock lock(baseLock);
	return socketsAllocated;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Peer::connect(const char* IP, int port)
{
	LogFunction(*network.getLog());
	Socket socket;

	network.getLog()->line(0,"creating socket");
	if ( (socket.socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP )) == INVALID_SOCKET)
	{
		Network::callError(Network::SocketCreation);
		network.getLog()->line(0,"error at socket(): %ld", network.getLastError() );
		return -1;
	}
	network.getLog()->line(0,"done");
	socket.state = Socket::Connecting;
	socket.addr.sin_family = AF_INET;
	socket.addr.sin_port = htons( port );
	socket.addr.sin_addr.s_addr = inet_addr(IP);
	return addSocket(socket);
}

void Peer::requestData(size_t slot, size_t amount)
{
	if( slot >= socketsAllocated )
	{
		Network::callError(Network::InvalidSlot);
		return;
	}
	sockets[slot].bytesPending = amount;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create listening socket for incoming connection
size_t Peer::listen( int port )
{
	LogFunction(*network.getLog());
	Socket s;
	if ( !IsInvalidSocket(s.socket = network.createSocket()))
	{
		// 2. bind socket
		network.getLog()->line(0,"binding a socket\n");
		sockaddr_in addr; // переменная для TCP сокета
		addr.sin_family = AF_INET;      // Семейство адресов - Internet
		addr.sin_port = htons (port);   // Назначаем 5001 порт сокету
		addr.sin_addr.s_addr = htonl (INADDR_ANY);   // Без конкретного адреса
		if (SocketError(bind(s.socket, (sockaddr*)&addr, sizeof(addr))))
		{
			network.getLog()->line(0,"-bind failed: %d\n", network.getLastError());
			network.closeSocket(s.socket);
			s.socket = INVALID_SOCKET;
			return -1;
		}
		else
		{
			network.getLog()->line(0,"Listening...\n");
			// 3. put it to listening mode
			if (SocketError(::listen( s.socket, maxClients )))
			{
				//ReportError(network.getLastError(), "Server listening failed");
				network.closeSocket(s.socket);
				s.socket = INVALID_SOCKET;
				return -1;
			}
		}
	}
	size_t result = -1;
	// 1. try to init listening socket
	if( s.socket == INVALID_SOCKET)
	{
		network.getLog()->line(0,"-socket failed: %ld\n", network.getLastError());
		//network.closeSocket(s.socket);
		return -1;
	}
	else
	{
		baseLock.lock();
		s.state = Socket::Accepting;
		result = addSocket(s);
		baseLock.unlock();
	}
	// 2. run thread for accepting clients
	return result;
}
///////////////////////////////////////////////////////////////////////////
// try to recieve incoming connection
bool Peer::processAccepting( size_t i )
{
	LogFunction(*network.getLog());
	Peer::Socket & s = sockets[i];
	// Accept a client socket
	Socket incoming;
	socklen_t addrLen = sizeof(s.addr);
	network.getLog()->line(0,"trying to accept in %s mode", s.blocking ? "blocking":"nonblocking");
	bool blocking = s.blocking;
	incoming.socket = accept(s.socket, (sockaddr*)&s.addr, &addrLen);
	if( incoming.socket == INVALID_SOCKET )
	{
		if( Network::getLastError() == Network::WouldBlock )
		{
		}
		else
			network.getLog()->line(0,"failed to accept");
		return false;
	}
	network.getLog()->line(0," accepted connection");
	incoming.state = Socket::Working;

	size_t newSocketID = addSocket(incoming);

	if( newSocketID >= 0)
	{
		if( !listener || !listener->onIncomingConnection( this, i, newSocketID ))
		{
			sockets[newSocketID].state = Socket::Dying;
		}
		return sockets[newSocketID].state != Socket::Dying;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////
// try to connect
void Peer::processConnecting( size_t i )
{
	LogFunction(*network.getLog());
	Peer::Socket & s = sockets[i];
	int res = 0;
	if (SocketError(res = ::connect( s.socket, (sockaddr*) &s.addr, sizeof(s.addr) )))
	{
		printf(".");
		//ReportError(WSAGetLastError(), "client connection failed");
	}
	else
	{
		s.state = Socket::Working;
		network.getLog()->line(0,"connected");
		if( listener )
			listener->onConnected( this, i );
	}
}
///////////////////////////////////////////////////////////////////////////
// try to recieve incoming message
void Peer::processRecieving( size_t i )
{
	Peer::Socket & s = sockets[i];
	int toRead = s.bytesPending - s.bytesRecieved;
	if( toRead > 0 )
	{
		if(buffer->size() < toRead )
			buffer->resize(toRead);
	
		int bytes = recv( s.socket, (char*)buffer->data(), toRead, 0);
		if( bytes < 0 )
		{
			network.getLog()->line(0,"Peer::processRecieving( %d ) - socket was closed\n",i);
			s.state = Socket::Dying;
			return;
		}
		if( bytes > 0 )
		{
			int recStart = s.bytesRecieved;
			int recTotal = s.bytesPending;
			
			s.bytesRecieved += bytes;
			if( s.bytesRecieved == s.bytesPending )
			{
				s.bytesRecieved = 0;
				s.bytesPending = 0;
			}	
			if( listener )
				listener->onRecieve( this, i, buffer->data(), bytes, recStart, recTotal);
			return;
		}
	}

	if( s.bytesRecieved == s.bytesPending )
	{
		s.bytesRecieved = 0;
		s.bytesPending = 0;
	}
	
}

void Peer::checkSockets(timeval &timeout)
{
	fd_set readfds, writefds, exceptfds;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	int awaits = 0;
	// 1. Fill socket sets
	for( size_t i = 0; i < socketsAllocated; ++i )
	{
		Socket & socket = sockets[i];
		if( socket.blocking == false )
		{
			sockets[i].toRead = true;
		}
		else if( (socket.state == Socket::Accepting || socket.state == Socket::Working))
		{
			FD_SET(socket.socket, &readfds);
			awaits++;
		}

		/*
		if( socket.state == Socket::Connecting )
		{
			FD_SET(socket.socket, &writefds);
		}*/
	}
	// 2. Mark sockets to be ready for reading
	int result = 0;
	if( awaits >= 0 )
	{
		result = select(0, &readfds, NULL, &exceptfds, &timeout);
		if( result > 0 )
			for( size_t i = 0; i < socketsAllocated; i++)
				if(sockets[i].blocking && FD_ISSET(sockets[i].socket, &readfds))
					sockets[i].toRead = true;
		else if( result == -1 )
		{
		    network.getLog()->line(0,"Select error");
			//ReportError(network.getLastError(), "client connection failed");
			return;
		}
	}
}
///////////////////////////////////////////////////////////////////////////
// update all sockets
void Peer::update(timeval &timeout)
{
	checkSockets(timeout);
	// 2. check sockets state


    // 3. process selected sockets
    size_t size = socketsAllocated;
    for( size_t i = 0; i < size; i++ )
    {
        Socket & s = sockets[i];
		/*
        if( result > 0 && FD_ISSET(s.socket, &exceptfds) )
        {
            network.getLog()->line(0,"Exception happened\n");
        }*/
        // check if socket was closed elsewhere
        if( s.socket == INVALID_SOCKET )
        {
            s.state = Socket::Empty;
            continue;
        }
        switch(s.state)
        {
        case Socket::Accepting:
            if( s.toRead )
                processAccepting(i);
            break;
        case Socket::Connecting:
            processConnecting(i);
            break;
        case Socket::Working:
            if( s.toRead )
                processRecieving(i);
            break;
        }
		s.toRead = false;
    }
	
	if( listener )
	{
	    for( int i = 0; i < socketsAllocated; i++)
            if( sockets[i].state == Socket::Dying)
                listener->onClosed(this, i);
	}

	// 4. remove dead sockets
	size = socketsAllocated;
	for( size_t i = 0; i < socketsAllocated; )
	{
		Socket & s = sockets[i];
		if(s.state == Socket::Dying )	// close socket and swap it with last socket
		{
			if( s.socket != INVALID_SOCKET )
			{
				network.closeSocket(s.socket);
				s.socket = INVALID_SOCKET;
			}

			sockets[i] = sockets[--size];
		}
		else
			i++;	// go to the next socket
	}

	if( size != socketsAllocated )
	{
		sockets = (Socket*)realloc(sockets, size * sizeof(Socket));
		if( sockets == NULL && size > 0 )
		{
			network.getLog()->line(0,"Allocation error: cannot resize sockets array");
			socketsAllocated = 0;
		}
		socketsAllocated = size;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
