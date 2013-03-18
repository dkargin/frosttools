#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NO_ADVANCED_STL
//#include "../../server/packets.h"

#ifdef WIN32
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include <frosttools/connection.h>

enum {
	CanRead = 1,
	CanWrite = 2,
	WasExcept = 4
};

int testSocket(int sockfd, int usec)
{
	fd_set readfds, writefds, exceptfds;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	FD_SET(sockfd, &readfds);
	FD_SET(sockfd, &exceptfds);
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = usec;
	int selected = select(FD_SETSIZE, &readfds, &writefds, &exceptfds, &timeout);

	int result = 0;

	if(selected > 0)
	{
		if(FD_ISSET(sockfd, &readfds))
			result |= CanRead;
		if(FD_ISSET(sockfd, &writefds))
			result |= CanWrite;
		if(FD_ISSET(sockfd, &exceptfds))
			result |= WasExcept;
	}

	return result;
}


const int InvalidSocket = -1;

Connection::Connection()
:ring(4096)
{
	ring.resize(4096);
	listener = NULL;
	netState = StateOffline;
	newState = StateOffline;
	error = 0;
	sockfd = InvalidSocket;
	storedSize = 0;
}

bool Connection::valid() const{
	return sockfd != InvalidSocket;
}

bool Connection::connect(const char *address, int port)
{
	if (netState == StateOffline)
	{
		writeLogI("Got cmd connect to %s:%d from StateOffline",address, port);
		strncpy(connectAddress,address, sizeof(connectAddress));
		connectPort = port;
		newState = StateConnecting;
		return true;
	}
	else if(netState == StateReady)
	{
		writeLogI("Got cmd connect to %s:%d from StateReady",address, port);
		return true;
	}
	writeLogI("Got cmd connect to %s:%d from StateConnecting",address, port);
	return false;
}

void Connection::disconnect()
{
	if(netState != StateReady)
		return;
	writeLogI("Got cmd disconnect from StateReady");
	newState = StateOffline;
}

int Connection::send(const void * data, int length)
{
	//LOGI("Connection::send writing %d bytes to buffer, available=%d, max=%d", length, ring.available(), ring.getMaxSize());
	if(ring.getMaxSize() - ring.available() >= length)
	{
		ring.writeForce((const char*)data, length);
		return length;
	}
	writeLogE("Connection::send buffer overflow");
	newState = StateOffline;
	return 0;
}

void Connection::processError(const char * where)
{
	//char message[255];
	//snprintf(message, 255, "%s: %s", strerror(errno));
	writeLogE("%s: %s", where, strerror(errno));
}
/// send all stored data
int Connection::handleSending()
{
	int bytesData = -1;
	int sendFlags = 0;
	sendFlags |= MSG_NOSIGNAL;

	while(ring.available() > 0)
	{
		bool done = false;
		size_t toWrite = ring.available();
		//LOGI("Connection::handleSending sending %d bytes", toWrite);
		/// request all data from buffer
		const char * data = ring.peek(toWrite);

		/// send obtained data
		while(!done)
		{
			int bytesData = ::send(sockfd, (const char*)data, toWrite, sendFlags);
			if(bytesData < 0)
			{
				if(errno == EAGAIN)
				{
					writeLogI("Connection::handleSending Operation to be blocked, trying after select");
					int test = testSocket(sockfd, 1000);
					if( test & WasExcept != 0)
					{
						writeLogE("Connection::handleSending Some exception happened");
						break;
					}
					if( test & CanWrite == 0)
					{
						writeLogE("Connection::send Cannot send data");
						break;
					}
				}
				else
				{
					writeLogE("Connection::handleSending Disconnected by host, err=%d,%s", errno, strerror(errno));
					disconnect();
					break;
				}
			}
			if(bytesData > 0)
			{
				if(bytesData == toWrite)
				{
					done = true;
				}
				else
				{
					writeLogE("Connection::handleSending partial send %d of %d bytes", errno, strerror(errno));
					toWrite -= bytesData;
					data += bytesData;
				}
			}
		}
	}
	return 1;
}

void Connection::handleStateChanged(NetworkState state) {
	NetworkState oldState = netState;

	netState = state;
	newState = state;
	this->storedSize = 0;
	ring.clean();
	/// TODO: call listener
	if(listener != NULL)
		listener->onStateChanged(state, oldState);
}

int Connection::handleDisconnect() {
	::close(sockfd);
	sockfd = InvalidSocket;
	return 1;
}

int setSocketNonblock(int sockfd, int block)
{
	/// set non-blocking mode
	int flags = fcntl(sockfd, F_GETFL, 0);
	if(block)
		flags = flags | O_NONBLOCK;
	else
		flags = flags & (~O_NONBLOCK);

	return fcntl(sockfd, F_SETFL, flags);
}

int Connection::handleStartConnection()
{
	hostent * server = gethostbyname(connectAddress);
	if(server == NULL)
	{
		writeLogE("Error : cannot get server address");
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		writeLogE("Error : Could not create socket");
		sockfd = InvalidSocket;
		return -1;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(connectPort);
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);

	//setSocketNonblock(sockfd, 1);
	writeLogI("Trying to connect to %s:%d",connectAddress, connectPort);
	int result = ::connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if(result < 0)
	{
		if(errno == EINPROGRESS || errno == EINTR )
		{
			writeLogE("handleStartConnection Connection would be tested later");
			return 0;
		}
		writeLogE("Error : Connection Failed, errno =%d", errno);
		close(sockfd);
		return -1;
	}
	writeLogI("handleStartConnection Connected to %s:%d",connectAddress, connectPort);
	return 1;
}

int Connection::handleConnect() {
	storedSize = 0;

	int connected = testSocket(sockfd, 10000);
	if(connected & CanRead != 0)
	{
		writeLogI("handleConnect Finally connected to %s:%d",connectAddress, connectPort);
		return 1;
	}
	if(connected & WasExcept != 0)
	{
		writeLogI("handleConnect Failed to connect to %s:%d",connectAddress, connectPort);
		return -1;
	}
	return 0;
}

int Connection::handleReceiving()
{
	int test = testSocket(sockfd, 10);
	if((test&CanRead) == 0)
		return 0;

	if(storedSize < bufferSize)
	{
		int flags = 0;

		int newBytes = ::recv(sockfd, buffer + storedSize, bufferSize - storedSize, flags);
		if(newBytes == 0)
		{
			// seems like shutdown is here - disconnect
			writeLogI("handleReceiving got 0 bytes - disconnected", errno, strerror(errno));
			return -1;
		}
		if(newBytes < 0) /// TODO: handle error here
		{
			if(errno == EWOULDBLOCK || errno == EAGAIN)
				return 0;
			writeLogE("handleReceiving error, err=%d,%s", errno, strerror(errno));
			return -1;
		}
		if(newBytes > 0)
		{
			storedSize += newBytes;
			if(listener != NULL)
			{
				int consumedSize = listener->onRead(buffer, storedSize);
				assert(consumedSize <= storedSize);
				if(consumedSize > 0)
				{
					memmove(buffer, buffer + consumedSize, storedSize - consumedSize);
					storedSize -= consumedSize;
				}
			}
		}
	}
	/*
	else
	{
		/// handle buffer overrun - maybe reconnect
		LOGE("handleReceiving Buffer overrun, total %d bytes stored - reconnecting", storedSize);
		storedSize = 0;
		handleStateChanged(StateConnecting);
	}*/
	return 0;
}

int Connection::update_network() {
	/// handle transitions
	if (newState != netState)
	{
		/// connect
		if (netState == StateOffline && newState == StateConnecting)
		{
			int res = handleStartConnection();
			if (res == 0)
				handleStateChanged(StateConnecting);
			else if(res > 0)
				handleStateChanged(StateReady);
			else
				handleStateChanged(StateOffline);
		}

		/// disconnect
		if (netState == StateReady && newState == StateOffline) {
			handleDisconnect();
			handleStateChanged(StateOffline);
		}

		/// reconnect
		if (netState == StateReady && newState == StateConnecting) {
			handleDisconnect();
			handleStateChanged(StateConnecting);		}
		if (newState == StateExit)
		{
			/// TODO: implement exit?
		}
	}

	if(netState == StateConnecting)
	{
		int result = handleConnect();
		if(result > 0)
			handleStateChanged(StateReady);
		if(result < 0)
			handleStateChanged(StateOffline);
	}

	if (netState == StateReady)
	{
		int result = handleReceiving();
		if(result < 0)
			handleStateChanged(StateOffline);
		result = handleSending();
		if(result < 0)
			handleStateChanged(StateOffline);
	}

	else
		if (netState == StateOffline)
		{
			/// TODO: implement offline
		}

	return netState;
}

