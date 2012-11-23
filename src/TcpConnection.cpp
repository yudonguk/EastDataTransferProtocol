#include "East/IO/Impl/TcpConnection.h"

#include <boost/bind.hpp>

TcpConnection::TcpConnection(boost::asio::io_service& io, IoConnectionHandler* connectionHandler)
	: mSocket(io), mSrand(io), mpConnectionHandler(connectionHandler), mState(DISCONNECTED)
{}

TcpConnection::~TcpConnection(void)
{
	Close();
}

void TcpConnection::RegisterConnectionHandler( IoConnectionHandler* pConnectionHandler )
{
	boost::unique_lock<boost::shared_mutex> writerLock(mSharedLockConnectionHandler);
	mpConnectionHandler = pConnectionHandler;

	if (mpConnectionHandler != NULL && mState == CONNECTED)
	{
		mSocket.get_io_service().post(
			mSrand.wrap(boost::bind(&IoConnectionHandler::OnConnected
			, mpConnectionHandler, boost::ref(*this))));
	}
}

IoConnectionHandler* TcpConnection::UnregisterConnectionHandler()
{
	boost::unique_lock<boost::shared_mutex> writerLock(mSharedLockConnectionHandler);
	auto previousConnectionHandler = mpConnectionHandler;
	mpConnectionHandler = NULL;
	return previousConnectionHandler;
}

void TcpConnection::AsyncWrite(const boost::asio::const_buffer& buffer )
{
	boost::asio::async_write(mSocket, boost::asio::buffer(buffer)
		, mSrand.wrap(boost::bind(&TcpConnection::AsnycWriteHandler
		, this, boost::asio::placeholders::bytes_transferred
		, boost::asio::placeholders::error)));
}

void TcpConnection::AsyncRead()
{
	Buffer* pBuffer = mPoolBuffer.construct();
	mSocket.async_receive(boost::asio::buffer(*pBuffer)
		, mSrand.wrap(boost::bind(&TcpConnection::AsyncReadHandler, this
		, pBuffer, boost::asio::placeholders::bytes_transferred
		, boost::asio::placeholders::error)));
}

size_t TcpConnection::Read( boost::asio::mutable_buffer& output )
{
	return mSocket.read_some(boost::asio::buffer(output));
}

size_t TcpConnection::Write( const boost::asio::const_buffer& input )
{
	return mSocket.write_some(boost::asio::buffer(input));
}

bool TcpConnection::Close()
{
	boost::system::error_code error;

	mSocket.shutdown(boost::asio::socket_base::shutdown_both, error);
	error.clear();
	mSocket.close(error);
	if (error)
	{
		return false;
	}

	boost::shared_lock<boost::shared_mutex> readerLock(mSharedLockConnectionHandler);
	if (mpConnectionHandler != nullptr && mState != DISCONNECTED)
	{
		mState = DISCONNECTED;
		mSocket.get_io_service().post(
			mSrand.wrap(boost::bind(&IoConnectionHandler::OnDisconnected
			, mpConnectionHandler, boost::ref(*this))));
	}

	return true;
}

void TcpConnection::AsyncReadHandler(Buffer* pBuffer, size_t readBytes, const boost::system::error_code& error )
{
	boost::shared_lock<boost::shared_mutex> readerLock(mSharedLockConnectionHandler);
	if (mpConnectionHandler == NULL)
	{
		mPoolBuffer.destroy(pBuffer);
		return;
	}

	if (error)
	{
		switch(error.value())
		{
		case boost::asio::error::eof:
			if (mState != DISCONNECTED)
			{
				mState = DISCONNECTED;
				mpConnectionHandler->OnDisconnected(*this);
			}
			break;
		case boost::asio::error::operation_aborted:
			//It is Canled Operation, so we will not do anything.
			break;
		default:
			if (mState != DISCONNECTED)
				mpConnectionHandler->OnError(*this, error);
			break;
		}
	}
	else
	{
		boost::asio::const_buffer readBuffer(pBuffer->data(), readBytes);
		mpConnectionHandler->OnRecevied(*this, readBuffer);
	}		

	mPoolBuffer.destroy(pBuffer);	
}

void TcpConnection::AsnycWriteHandler( size_t sendtBytes, const boost::system::error_code& error )
{
	boost::shared_lock<boost::shared_mutex> readerLock(mSharedLockConnectionHandler);
	if (mpConnectionHandler == NULL)
	{
		return;
	}

	if (error)
	{
		switch(error.value())
		{
		case boost::asio::error::eof:
			if (mState != DISCONNECTED)
			{
				mState = DISCONNECTED;
				mpConnectionHandler->OnDisconnected(*this);
			}
			break;
		case boost::asio::error::operation_aborted:
			//It is Canled Operation, so we will not do anything.
			break;
		default:
			if (mState != DISCONNECTED)
				mpConnectionHandler->OnError(*this, error);
			break;
		}
	}
	else
	{
		mpConnectionHandler->OnSent(*this);
	}
}
