#pragma once

#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/array.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "East/IO/IoConnection.h"

class TcpConnection : public IoConnection, public IoSocket
{
public:
	friend class TcpServerConnector;
	friend class TcpClientConnector;

public:
	typedef boost::array<uint8_t, 1024 * 4> Buffer;
	typedef boost::asio::ip::tcp::socket SocketType;
		
private:
	enum ConnectionState 
	{
		CONNECTED, DISCONNECTED
	};

public:
	TcpConnection(boost::asio::io_service& io, IoConnectionHandler* connectionHandler = NULL);
	~TcpConnection(void);

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
	void AsyncReadHandler(Buffer* pBuffer, size_t readBytes, const boost::system::error_code& error);
	void AsnycWriteHandler(size_t sendtBytes, const boost::system::error_code& error);

private:
	boost::shared_mutex mSharedLockConnectionHandler;
	IoConnectionHandler* mpConnectionHandler;
	SocketType mSocket;

	boost::object_pool<Buffer> mPoolBuffer;

	boost::asio::io_service::strand mSrand;
	ConnectionState mState;
};
