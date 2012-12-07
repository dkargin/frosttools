//#include "stdafx.h"
#include <frosttools/network.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif

const size_t DEFAULT_BUFLEN = 0xffff;

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
	case BufferOverrun:
		return "Input buffer overrun";
	}
	return "Unknown error";
}
//typedef unsigned int (__stdcall *ThreadFunction)(void *);
#ifdef _MSC_VER
void ReportErrorWin32(int errorCode, const char *whichFunc)
{
	char errorMsg[500];          // Declare a buffer to hold the generated messages
	memset(errorMsg, sizeof(errorMsg),0);            // Automatically NULL-terminate the strings

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

	//	fprintf(stderr, errorMsg);
	return;
};
#endif

bool IsInvalidSocket(Network::SOCKET socket)
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
	/*
	/// WTF is this?
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
	}*/
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
#ifdef _MSC_VER
int Network::setBlocking(Network::SOCKET s, bool value)
{
	/// TODO: Implement it
	return 0;
}
int Network::closeSocket(SOCKET socket)
{
	return closesocket(socket);	
}
#else
int Network::setBlocking(Network::SOCKET s, bool value)
{
	int oldValue = fcntl(s, F_GETFL);

	if(value && fcntl(s, F_SETFL, oldValue & ~O_NONBLOCK) < 0)
	{		
		printf("Network::setBlocking() : Cannot enter blocking mode\n");
		return -1;
	}
	else if(!value && fcntl(s, F_SETFL, oldValue | O_NONBLOCK) < 0)
	{
		printf("Network::setBlocking() : Cannot enter nonblocking mode\n");
		return -1;
	}
	return 0;
}
int Network::closeSocket(SOCKET socket)
{
	return close(socket);
}
#endif

Network::SOCKET Network::createSocket(SocketType socketType)
{
	LogFunction(*log);
	Network::SOCKET result = INVALID_SOCKET;
	char flag = 0;
	int ret = 0;
	switch(socketType)
	{
	case SocketTCP:
		result = socket(AF_INET, SOCK_STREAM, 0);
		flag = 1;
		// no delay for TCP
		ret = setsockopt( result, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
		if(ret == -1)
		{
			printf("Network::createSocket() cannot turn on nodelay\n");
		}
		// Put the socket in non-blocking mode:		
		break;
	case SocketUDP:
		result = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		break;
	}
	return result;
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
	Peer::SlotState state;
	Network::SOCKET socket;
	sockaddr_in addr;
	size_t id;
	Network::SocketType socketType;
	//int bytesPending;
	int bytesRecieved;
	bool manualRead;
	unsigned char * buffer;
	size_t bufferLength;
	//bool toRead;	// if socket is ready for read operations
	SlotUpdateMode mode;	// if socket is in nonblocking mode
	Socket()
	{
		socketType = Network::SocketTCP;
		id = -1;
		state = Empty;
		buffer = NULL;
		bufferLength = 0;
		bytesRecieved = 0;
		socket = INVALID_SOCKET;
		mode = ModeAsyncSelect;
		manualRead = false;
	}

	void release()
	{
		free(buffer);
		buffer = NULL;
	}
	void resizeBuffer(size_t newLength)
	{
		buffer = (unsigned char*)realloc(buffer, newLength);
		bufferLength = buffer ? newLength : 0;
	}
	//Socket(const Socket & s) : state(s.state), manualRead(s.manualRead), socket(s.socket), addr(s.addr), bytesRecieved(s.bytesRecieved), mode(s.mode), id(s.id){}
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Peer::Peer(Network &net, int bufferSize)
	:listener(NULL), network(net), sockets(NULL), socketsAllocated(0)
{
	buffer = new char[DEFAULT_BUFLEN];
	if(buffer)
		bufferLength = DEFAULT_BUFLEN;
	//buffer->resize(bufferSize);
	dying = false;
	reconnect = true;
}

Peer::~Peer()
{
	close();
	if(buffer)
	{
		delete[] buffer;
		buffer = NULL;
		bufferLength = 0;
	}
}

void Peer::close()
{
	for( size_t i = 0; i < socketsAllocated; i++)
	{
		network.closeSocket(sockets[i].socket);
		sockets[i].release();
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
	if( s.state != Working )
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
			s.state = Dying;
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
	/// search for the first empty socket
	for(size_t i = 0; i < socketsAllocated; i++)
	{
		if(sockets[i].state == Empty)
		{
			memcpy(sockets+i, &s, sizeof(Socket));
			return i;
		}
	}

	/// or reallocate socket array for new slot
	size_t newSize = socketsAllocated + 1;

	sockets = (Socket*)realloc(sockets, newSize * sizeof(Socket));
	/// cannot realloc - memory error
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
void Peer::setNoDelay(size_t slot, bool value)
{
	if( slot >= socketsAllocated )
	{
		Network::callError(Network::InvalidSlot);
		return;
	}
	int flag = value ? 1: 0;
	int ret = setsockopt( sockets[slot].socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	if(ret == -1)
	{
		printf("Peer::setNoDelay cannot turn on nodelay, errno=%d, err=%s\n", errno, strerror(errno));
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Peer::setSlotMode(size_t slot, SlotUpdateMode mode)
{
	if( slot >= socketsAllocated )
	{
		Network::callError(Network::InvalidSlot);
		return;
	}
	if( sockets[slot].mode != mode)
	{
		int flags;
		switch(mode)
		{
		case ModeManual:
			break;
		case ModeAsyncSimple:
#if _WIN32
#else
			flags = fcntl(sockets[slot].socket,F_GETFL,0);
			assert(flags != -1);
			fcntl(sockets[slot].socket, F_SETFL, flags | O_NONBLOCK);
#endif
			break;
		case ModeAsyncSelect:
#if _WIN32
#else
			/*
			flags = fcntl(sockets[slot].socket,F_GETFL,0);
			assert(flags != -1);
			fcntl(sockets[slot].socket, F_SETFL, flags & ~O_NONBLOCK);*/
#endif
			break;
		}
		/*
		#if _MSC_VER
		#else
		int flags;
		flags = fcntl(sockets[slot].socket,F_GETFL,0);
		assert(flags != -1);
		fcntl(sockets[slot].socket, F_SETFL, flags | O_NONBLOCK);
		#endif
		*/
		sockets[slot].mode = mode;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Peer::getClientsUsed() const
{
	Lock lock((Mutex&)baseLock);
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
	socket.state = Connecting;
	socket.addr.sin_family = AF_INET;
	socket.addr.sin_port = htons( port );
	socket.addr.sin_addr.s_addr = inet_addr(IP);
	return addSocket(socket);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create listening socket for incoming connection
size_t Peer::listen( int port )
{
	LogFunction(*network.getLog());
	Socket s;
	if ( !IsInvalidSocket(s.socket = network.createSocket(Network::SocketTCP)))
	{
#ifdef _WIN32
#else
		int reuse_addr = 1;
		setsockopt(s.socket, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
#endif
		// 2. bind socket
		network.getLog()->line(0,"binding a socket\n");
		sockaddr_in addr; //
		addr.sin_family = AF_INET;      //
		addr.sin_port = htons (port);   //
		addr.sin_addr.s_addr = htonl (INADDR_ANY);   //
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
		s.state = Accepting;
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
	const char * modeName[] = {"ModeManual", "ModeAsyncSimple", "ModeAsyncSelect"};
	LogFunction(*network.getLog());
	Peer::Socket & s = sockets[i];
	// Accept a client socket
	Socket incoming;
	socklen_t addrLen = sizeof(s.addr);
	network.getLog()->line(0,"trying to accept slot %d in %s mode", i, modeName[s.mode]);

	incoming.socket = accept(s.socket, (sockaddr*)&s.addr, &addrLen);
	if( incoming.socket == INVALID_SOCKET )
	{
		if( Network::getLastError() == Network::WouldBlock )
		{
			printf("Peer::processAccepting: would block");
		}
		else
			network.getLog()->line(0,"failed to accept");
		return false;
	}
	network.getLog()->line(0," accepted connection");
	incoming.state = Working;

	size_t newSocketID = addSocket(incoming);

	if( newSocketID >= 0)
	{
		if( !listener || !listener->onIncomingConnection( this, i, newSocketID ))
		{
			sockets[newSocketID].state = Dying;
		}
		return sockets[newSocketID].state != Dying;
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
		s.state = Working;
		network.getLog()->line(0,"connected");
		// no delay for TCP
		int flag = 1;
		int ret = setsockopt( s.socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
		if(ret == -1)
		{
			printf("Peer::processConnecting() cannot turn on nodelay, errno=%d, err=%s\n", errno, strerror(errno));
		}
		if( listener )
			listener->onConnected( this, i );
	}
}
///////////////////////////////////////////////////////////////////////////
// Manually obtain buffered data from slot
int Peer::recv(size_t peerId, void * buffer, size_t maxLength)
{
	if(peerId >= maxClients)
	{
		Network::callError(Network::InvalidSlot);
		return -1;
	}

	Peer::Socket & s = sockets[peerId];

	if(s.state == Working)
	{
		return ::recv(s.socket, (char*)buffer, maxLength, 0 );
	}//if(s.)
	size_t dataToProcess = s.bytesRecieved;
	if(maxLength < dataToProcess)
		dataToProcess = maxLength;
	//memcpy(buffer, )
	/// TODO: implement
	return 0;
}

int Peer::getDataPending(size_t peerId) const
{
	if(peerId >= maxClients || sockets[peerId].state != Working)
	{
		Network::callError(Network::InvalidSlot);
		return -1;
	}
	return sockets[peerId].bytesRecieved;
}
///////////////////////////////////////////////////////////////////////////
// try to receive incoming message
void Peer::processRecieving( size_t i )
{
	Peer::Socket & s = sockets[i];
	if(s.manualRead)
	{
		s.bytesRecieved = 1;
		return;
	}
	if( s.bufferLength == 0)
	{
		s.resizeBuffer(bufferLength);
	}
	//size_t toRead = s.bytesPending - s.bytesRecieved;
	//size_t totalData = s.bytesRecieved;
	if( s.bufferLength >= s.bytesRecieved)
	{
		int bytes = ::recv( s.socket, (char*)s.buffer + s.bytesRecieved, s.bufferLength - s.bytesRecieved, 0);
		if( bytes <= 0 )
		{
			network.getLog()->line(0,"Peer::processRecieving( %d ) - socket was closed\n",i);
			s.state = Dying;
			return;
		}
		s.bytesRecieved += bytes;
	}	
	else
	{
		Network::callError(Network::BufferOverrun);
		s.state = Dying;
		return;
		//printf("No place to store data new data\n");
	}

	if(s.bytesRecieved > 0 && listener)
	{
		int processed = listener->onRecieve( this, i, s.buffer, s.bytesRecieved);
		int unreceivedData = s.bytesRecieved - processed;
		memmove(s.buffer, s.buffer + processed, unreceivedData);
		s.bytesRecieved = unreceivedData;
	}
}
///////////////////////////////////////////////////////////////////////////
// update all sockets
void Peer::update(timeval &timeout)
{
	fd_set readfds, writefds, exceptfds;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	int selectAwaits = 0;
	int selected = 0;
	// 1. Fill socket sets
	for( size_t i = 0; i < socketsAllocated; ++i )
	{
		Socket & s = sockets[i];
		if( s.mode == ModeAsyncSelect && (s.state == Accepting || s.state == Working))
		{
			FD_SET(s.socket, &readfds);
			selectAwaits++;
		}
		/*
		if( socket.state == Socket::Connecting )
		{
		FD_SET(socket.socket, &writefds);
		}*/
	}
	if( selectAwaits > 0 )
		selected = select(FD_SETSIZE, &readfds, NULL, &exceptfds, &timeout);

	if( selected < 0 )
	{
		network.getLog()->line(0,"Select error");
		//ReportError(network.getLastError(), "client connection failed");
		//return;
	}
	// 2. Process all sockets
	for( size_t i = 0; i < socketsAllocated; i++)
	{
		Socket & s = sockets[i];
		if(s.mode == ModeManual || s.state == Empty)
			continue;
		/// check if socket was updatet through "select"
		if(s.mode == ModeAsyncSelect)
		{
			if( s.state == Connecting )
				processConnecting(i);
			else if(selected > 0 && FD_ISSET(s.socket, &readfds))
			{
				if( s.state == Accepting )
					processAccepting(i);
				else if(s.state == Working)
					processRecieving(i);
			}
		}
		else if(s.mode == ModeAsyncSimple)
		{
			switch(s.state)
			{
			case Accepting:		processAccepting(i);	break;
			case Connecting:	processConnecting(i);	break;
			case Working:		processRecieving(i);	break;
			}
		}

		if( s.socket == INVALID_SOCKET )
		{
			s.state = Empty;
		}
	}
	// 2. Mark sockets to be ready for reading

	// 2. check sockets state

	// 3. process selected sockets

	if( listener )
	{
		for( size_t i = 0; i < socketsAllocated; i++)
			if( sockets[i].state == Dying )
				listener->onClosed(this, i);
	}

	for( size_t i = 0; i < socketsAllocated; i++)
	{
		if( sockets[i].state == Dying)
			sockets[i].state = Empty;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* mkaddr.c
* The mkaddr() Subroutine using inet_aton
* Make a socket address:
*/
#include <stdio.h>

#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
/*
* Create an AF_INET Address:
*
* ARGUMENTS:
* 1. addr Ptr to area
* where address is
* to be placed.
* 2. addrlen Ptr to int that
* will hold the final
* address length.
* 3. str_addr The input string
* format hostname, and
* port.
* 4. protocol The input string
* indicating the
* protocol being used.
* NULL implies  tcp .
* RETURNS:
* 0 Success.
* -1 Bad host part.
* -2 Bad port part.
*
* NOTES:
*  *  for the host portion of the
* address implies INADDR_ANY.
*
*  *  for the port portion will
* imply zero for the port (assign
* a port number).
*
* EXAMPLES:
*  www.lwn.net:80
*  localhost:telnet
*  *:21
*  *:*
*  ftp.redhat.com:ftp
*  sunsite.unc.edu
*  sunsite.unc.edu:*
*/
int mkaddr(void *addr, int *addrlen, const char *str_addr, const char *protocol) {

	char *inp_addr = strdup(str_addr);
	char *host_part = strtok(inp_addr, ":");
	char *port_part = strtok(NULL, "\n");
	struct sockaddr_in *ap = (struct sockaddr_in *) addr;
	struct hostent *hp = NULL;
	struct servent *sp = NULL;
	char *cp;
	long lv;

	/*
	* Set input defaults:
	*/
	if (!host_part) {
		host_part = "*";
	}
	if (!port_part) {
		port_part = "*";
	}
	if (!protocol) {
		protocol = "tcp";
	}

	/*
	* Initialize the address structure:
	*/
	memset(ap, 0, *addrlen);
	ap->sin_family = AF_INET;
	ap->sin_port = 0;
	ap->sin_addr.s_addr = INADDR_ANY;

	/*
	* Fill in the host address:
	*/
	if (strcmp(host_part, "*") == 0) {
		; /* Leave as INADDR_ANY */
	} 
	else if (isdigit(*host_part)) {
		/*
		* Numeric IP address:
		*/
		ap->sin_addr.s_addr = inet_addr(host_part);
		assert( ap->sin_addr.s_addr != INADDR_NONE );
		if( ap->sin_addr.s_addr == INADDR_NONE )
			return -1;
		/*
		if (!inet_aton(host_part, &ap->sin_addr)) {
			return -1;
		}*/
	}
	else 
	{
		/*
		* Assume a hostname:
		*/
		hp = gethostbyname(host_part);
		if (!hp) {
			return -1;
		}
		if (hp->h_addrtype != AF_INET) {
			return -1;
		}
		ap->sin_addr = *(struct in_addr *) hp->h_addr_list[0];
	}

	/*
	* Process an optional port #:
	*/
	if (!strcmp(port_part, "*")) {
		/* Leave as wild (zero) */
	} else if (isdigit(*port_part)) {
		/*
		* Process numeric port #:
		*/
		lv = strtol(port_part, &cp, 10);
		if (cp != NULL && *cp) {
			return -2;
		}
		if (lv < 0L || lv >= 32768) {
			return -2;
		}
		ap->sin_port = htons((short) lv);
	} else {
		/*
		* Lookup the service:
		*/
		sp = getservbyname(port_part, protocol);
		if (!sp) {
			return -2;
		}
		ap->sin_port = (short) sp->s_port;
	}

	/*
	* Return address length
	*/
	*addrlen = sizeof *ap;

	free(inp_addr);
	return 0;
}

const char msgSignature[]="SeDi";

ServiceDesc::ServiceDesc()
{
	/// TODO: dinosaur can bite here
	memset(this, sizeof(ServiceDesc), 0);
	memcpy(this->signature, msgSignature, 4);
}

ServiceDesc::ServiceDesc(const ServiceDesc &desc)
{
	memcpy(this, &desc, sizeof(desc));
}


BroadcasterData::BroadcasterData()
{
	broadcastAddress = "192.168.1.255:9097";
	timeout = 1000;
}

void BroadcasterData::addService(const ServiceDesc & desc)
{
	services.push_back(desc);
}

void sendServiceDesc(const ServiceDesc & desc, Network::SOCKET socket, sockaddr_in & address)
{
	int packetLength = sizeof(ServiceDesc);
	int addrLength = sizeof(sockaddr_in);
	int err = sendto(socket, (const char*)&desc, packetLength,0, (sockaddr*)&address, addrLength);
	if(err < 0)
	{
		err = errno;
		//char errorBuff[255];
		//strerror_r(err, errorBuff, sizeof(errorBuff));
		printf("Failed to send service descriptor, err = %d, %s\n", err, strerror(err));
	}
}

void broadcastServices(BroadcasterData::Services & services, Network::SOCKET socket, sockaddr_in & address)
{
	for(BroadcasterData::Services::iterator it = services.begin(); it != services.end(); ++it)
	{
		ServiceDesc & desc = *it;
		sendServiceDesc(desc, socket, address);
	}
}

void run_broadcast(Network & network, BroadcasterData * br)
{
	Network::SOCKET socket = network.createSocket(Network::SocketUDP);
	if (socket == INVALID_SOCKET) {
		printf("Cannot create broadcaster\n");
		exit(0);
	}

	char sv_addr[128] = "127.0.0:*";

	sockaddr_in broadcast_address, server_address;
	memset((char*) &broadcast_address, sizeof(broadcast_address), 0);

	int len_bc = sizeof broadcast_address;
	int z = mkaddr(&broadcast_address, &len_bc, br->broadcastAddress.c_str(), "udp"); /* UDP protocol */
	if(z == -1)
	{
		printf("failed to init bc_addr\n");
	}

	int len_srvr = sizeof server_address;
	z = mkaddr(&server_address, &len_srvr, sv_addr, "udp"); /* UDP protocol */

	if(z == -1)
	{
		printf("failed to init sv_addr\n");
	}

	int option = 1;
	if ((z = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR,
		(char*)&option, sizeof option)) == -1) {
			printf("Cannot set SO_REUSEADDR=1\n");
			exit(0);
	}
	option = 1;
	if ((z = setsockopt(socket, SOL_SOCKET, SO_BROADCAST,
		(char*)&option, sizeof option)) == -1) {
			printf("Cannot set broadcasting mode\n");
			exit(0);
	}

	if (bind(socket, (sockaddr*) &broadcast_address,
		sizeof(broadcast_address)) == -1) {
			printf("Cannot bind broadcaster\n");
			exit(0);
	}

	while (true)
	{
		broadcastServices(br->services, socket, broadcast_address);
		Threading::Thread::sleep(br->timeout);		
	}
}

Network::SOCKET init_broadcast(Network &network, BroadcasterData *br, sockaddr_in &broadcast_address)
{
	Network::SOCKET socket = network.createSocket(Network::SocketUDP);
	if (socket == INVALID_SOCKET) {
		printf("Cannot create broadcaster\n");
		return INVALID_SOCKET; //exit(0);
	}

	char sv_addr[128] = "127.0.0:*";

	//sockaddr_in broadcast_address, server_address;
	memset((char*) &broadcast_address, sizeof(broadcast_address), 0);

	int len_bc = sizeof broadcast_address;
	int z = mkaddr(&broadcast_address, &len_bc, br->broadcastAddress.c_str(), "udp"); /* UDP protocol */
	if(z == -1)
	{
		printf("failed to init bc_addr\n");
	}

	/*int len_srvr = sizeof server_address;
	z = mkaddr(&server_address, &len_srvr, sv_addr, "udp"); //* UDP protocol * /
	if(z == -1)
	{
		printf("failed to init sv_addr\n");
	}*/

	int option = 1;
	if ((z = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR,
		(char*)&option, sizeof option)) == -1) {
			printf("Cannot set SO_REUSEADDR=1\n");
			return INVALID_SOCKET; //exit(0);
	}
	option = 1;
	if ((z = setsockopt(socket, SOL_SOCKET, SO_BROADCAST,
		(char*)&option, sizeof option)) == -1) {
			printf("Cannot set broadcasting mode\n");
			return INVALID_SOCKET; //exit(0);
	}

	if (bind(socket, (sockaddr*) &broadcast_address,
		sizeof(broadcast_address)) == -1) {
			printf("Cannot bind broadcaster\n");
			return INVALID_SOCKET; //exit(0);
	}
	return socket;
}

void run_broadcast_once(Network::SOCKET &socket, BroadcasterData *br, sockaddr_in broadcast_address) {
	//while (true)
	{
		broadcastServices(br->services, socket, broadcast_address);
		//Threading::Thread::sleep(br->timeout);
	}
}
