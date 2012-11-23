#pragma once

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/array.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "East/IO/IoConnection.h"

class UdpConnection : public IoConnection, public IoSocket
{
public:
	friend class UdpServerConnector;
	friend class UdpClientConnector;

public:
	//65507 is max payload of packet size in udp.
	typedef boost::array<uint8_t, 65507> Buffer;
	typedef boost::asio::ip::udp::socket SocketType;

private:
	enum ConnectionState 
	{
		CONNECTED, DISCONNECTED
	};

public:
	UdpConnection(boost::asio::io_service& io, IoConnectionHandler* connectionHandler = NULL);
	~UdpConnection(void);

public:
	void RegisterConnectionHandler(IoConnectionHandler* pConnectionHandler);
	IoConnectionHandler* UnregisterConnectionHandler();

public:
	void AsyncRead();
	void AsyncWrite(const boost::asio::const_buffer& buffer);
	size_t Read(boost::asio::mutable_buffer& output);
	size_t Write(const boost::asio::const_buffer& input);
	bool Close();

private:
	void AsyncReadHandler(Buffer* pBuffer, size_t readBytes, boost::asio::ip::udp::endpoint* pRemoteEndpoint, const boost::system::error_code& error);
	void AsnycWriteHandler(size_t sendtBytes, const boost::system::error_code& error);
	
private:
	boost::shared_mutex mSharedLockConnectionHandler;
	IoConnectionHandler* mpConnectionHandler;
	ConnectionState mState;
	
	SocketType mSocket;
	SocketType::endpoint_type mRemoteEndpoint;

	boost::object_pool<Buffer> mPoolBuffer;
	boost::object_pool<boost::asio::ip::udp::endpoint> mPoolEnpoint;

	boost::asio::io_service::strand mSrand;	
};
