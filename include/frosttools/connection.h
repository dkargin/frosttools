#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <stdio.h>
#include <sys/types.h>


#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <frosttools/ringbuffer.hpp>
#include <frosttools/threads.hpp>

/*
/// Seems like it is used by android NativeNet module. Noone else uses it
#ifdef ANDROID_LOG
#include <android/log.h>
#define  LOG_TAG    "NativeNet"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#elif CONNECTION_NOLOG
#define  LOGI(...)
#define  LOGE(...)
#endif
*/

namespace frosttools
{
/// Connection
class Connection
{
public:
	/// Ringbuffer to store data
	RingBuffer ring;
	/// Network state
	enum NetworkState {
		StateExit = -1, StateOffline, StateConnecting, StateReady
	};

	/// Network event listener
	class Listener
	{
	public:
		virtual ~Listener() {}
		/// Called when network state has changed
		virtual void onStateChanged(NetworkState newState, NetworkState oldState) = 0;
		/// Called when incoming data is handled
		virtual int onRead(const char * data, int length) = 0;
	};

	/// Network state
	NetworkState netState, newState;

	/// Endpoint address
	char connectAddress[255];
	/// Endpoint port
	size_t connectPort;

	/// Last error code
	int error;
	/// Error code
	enum ErrorType
	{
		ErrSocket,	// could not create socket
		ErrAddress, // could not resolve address
		ErrOverflow, // buffer overflow
	};

	/// Listener
	Listener * listener;

	/// Socket file desriptor
	int sockfd;

	enum{bufferSize = 0xffff};
	/// Data buffer to store incoming packets
	char buffer[bufferSize];
	/// Stored data size
	int storedSize;

	Connection();
	/// Check if connection is valid
	bool valid() const;
	/// Connec to endpoint
	bool connect(const char *address, int port);
	/// Disconnect
	void disconnect();
	/// Update network
	int update_network();

	/// Send data to endpoint
	int send(const void * data, int length);
protected:
	/// Write to log
	void writeLog(int level, const char * format, ...);
	/// Write to info log
	virtual void writeLogI(const char * format, ...) = 0;
	/// Write to error log
	virtual void writeLogE(const char * format, ...) = 0;

	/// Process error
	void processError(const char * where);
	/// Handle state transition
	void handleStateChanged(NetworkState state);
	/// -1 = failed
	/// 0 = continue
	/// 1 = done
	/// Handle disconnect
	int handleDisconnect();
	/// Handle connection start
	int handleStartConnection();
	/// Handle connection
	int handleConnect();
	/// Handle data sending
	int handleSending();
	/// Handle data receiving
	int handleReceiving();
};

}

#endif
