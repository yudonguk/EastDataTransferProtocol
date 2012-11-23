#include "East/IO/Impl/UdpConnection.h"

#include <boost/bind.hpp>

UdpConnection::UdpConnection(boost::asio::io_service& io, IoConnectionHandler* connectionHandler)
	: mSocket(io), mSrand(io), mpConnectionHandler(connectionHandler), mState(DISCONNECTED)
{
}

UdpConnection::~UdpConnection(void)
{
	Close();
}

void UdpConnection::RegisterConnectionHandler( IoConnectionHandler* pConnectionHandler )
{
	boost::unique_lock<boost::shared_mutex> writerLock(mSharedLockConnectionHandler);
	mpConnectionHandler = pConnectionHandler;
	if (mpConnectionHandler != NULL	&& mState == CONNECTED)
	{
		mSocket.get_io_service().post(
			mSrand.wrap(boost::bind(&IoConnectionHandler::OnConnected
			, mpConnectionHandler, boost::ref(*this))));
	}
}

IoConnectionHandler* UdpConnection::UnregisterConnectionHandler()
{
	boost::unique_lock<boost::shared_mutex> writerLock(mSharedLockConnectionHandler);
	auto previousConnectionHandler = mpConnectionHandler;
	mpConnectionHandler = NULL;
	return previousConnectionHandler;
}

void UdpConnection::AsyncWrite(const boost::asio::const_buffer& buffer )
{
	mSocket.async_send_to(boost::asio::buffer(buffer), mRemoteEndpoint
		, mSrand.wrap(boost::bind(&UdpConnection::AsnycWriteHandler
		, this, boost::asio::placeholders::bytes_transferred
		, boost::asio::placeholders::error)));
}

void UdpConnection::AsyncRead()
{
	Buffer* pBuffer = mPoolBuffer.construct();
	auto* pRemoteEndpoint = mPoolEnpoint.construct();
	mSocket.async_receive_from(boost::asio::buffer(*pBuffer), *pRemoteEndpoint
		, mSrand.wrap(boost::bind(&UdpConnection::AsyncReadHandler, this
		, pBuffer, boost::asio::placeholders::bytes_transferred, pRemoteEndpoint
		, boost::asio::placeholders::error)));
}

size_t UdpConnection::Read( boost::asio::mutable_buffer& output )
{
	boost::asio::ip::udp::endpoint remoteEndopoint;
	size_t receivedSize = mSocket.receive_from(boost::asio::buffer(output), remoteEndopoint);

	if (remoteEndopoint != mRemoteEndpoint)
	{
		return 0;
	}

	return receivedSize;
}

size_t UdpConnection::Write( const boost::asio::const_buffer& input )
{
	return mSocket.send_to(boost::asio::buffer(input), mRemoteEndpoint);
}

bool UdpConnection::Close()
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

void UdpConnection::AsyncReadHandler(Buffer* pBuffer, size_t readBytes, boost::asio::ip::udp::endpoint* pRemoteEndpoint, const boost::system::error_code& error )
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
		if (mRemoteEndpoint == *pRemoteEndpoint)
		{
			boost::asio::const_buffer readBuffer(pBuffer->data(), readBytes);
			mpConnectionHandler->OnRecevied(*this, readBuffer);
		}
		else
		{
			AsyncRead();
		}
	}	

	mPoolEnpoint.destroy(pRemoteEndpoint);
	mPoolBuffer.destroy(pBuffer);	
}

void UdpConnection::AsnycWriteHandler( size_t sendtBytes, const boost::system::error_code& error )
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

