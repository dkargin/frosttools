#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../../frostTools/ringbuffer.hpp"
#include "../../frostTools/threads.hpp"
class Connection {
public:
	RingBuffer ring;
	enum NetworkState {
		StateExit = -1, StateOffline, StateConnecting, StateReady
	};

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void onStateChanged(NetworkState newState, NetworkState oldState) = 0;
		virtual int onRead(const char * data, int length) = 0;
	};

	NetworkState netState, newState;

	char connectAddress[255];
	size_t connectPort;

	int error;
	enum ErrorType
	{
		ErrSocket,	// could not create socket
		ErrAddress, // could not resolve address
		ErrOverflow, // buffer overflow
	};

	Listener * listener;

	int sockfd;
	enum{bufferSize = 4096};
	char buffer[bufferSize];
	int storedSize;

	Connection();
	bool valid() const;
	bool connect(const char *address, int port);
	void disconnect();
	int update_network();

	int send(const void * data, int length);
protected:
	void handleStateChanged(NetworkState state);
	/// -1 = failed
	/// 0 = continue
	/// 1 = done
	int handleDisconnect();
	int handleStartConnection();
	int handleConnect();
	int handleSending();
	int handleReceiving();
};

#endif
