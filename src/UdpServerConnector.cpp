#include "East/IO/Impl/UdpServerConnector.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

#include "East/IO/IoServiceManager.h"
#include "East/IO/Impl/UdpConnection.h"

UdpServerConnector::UdpServerConnector( IoServiceManager& ioServiceManager, uint16_t port)
	: mIoServiceManager(ioServiceManager), mPort(port)
{}

UdpServerConnector::~UdpServerConnector()
{
}

IoConnection* UdpServerConnector::Connect( IoConnectionHandler* ioConnectionHandler )
{
	UdpConnection* pConnection = new UdpConnection(mIoServiceManager.GetIoService(), ioConnectionHandler);

	using namespace boost::asio::ip;

	auto& socket = pConnection->mSocket;

	boost::system::error_code error;
	socket.bind(udp::endpoint(udp::v4(), mPort), error);

	if (error)
	{
		delete pConnection;
		pConnection = nullptr;
		return nullptr;
	}

	static boost::function<void(UdpConnection*, UdpConnection::Buffer*, size_t, const boost::system::error_code&)> asyncReadHandler = 
	[](UdpConnection* pConnection, UdpConnection::Buffer* pBuffer, size_t readBytes, const boost::system::error_code& error)
	{
		boost::shared_lock<boost::shared_mutex> readerLock(pConnection->mSharedLockConnectionHandler);

		IoConnectionHandler* pConnectionHandler = pConnection->mpConnectionHandler;

		if (!error)
		{
			pConnection->mState = UdpConnection::CONNECTED;
			if (pConnectionHandler != nullptr)
			{		
				pConnection->mSocket.get_io_service().post(
					pConnection->mSrand.wrap(boost::bind(&IoConnectionHandler::OnConnected
					, pConnectionHandler, boost::ref(*pConnection))));

				pConnection->mSocket.get_io_service().post(
					pConnection->mSrand.wrap(boost::bind(&UdpConnection::AsyncReadHandler, pConnection
					, pBuffer, readBytes, pConnection->mPoolEnpoint.construct(pConnection->mRemoteEndpoint)
					, boost::system::error_code())));
			}
		}
		else
		{
			pConnection->mState = UdpConnection::DISCONNECTED;
			if (pConnectionHandler != nullptr)
			{
				pConnectionHandler->OnDisconnected(*pConnection);
			}
		}		
	};

	auto* pBuffer = pConnection->mPoolBuffer.construct();
	socket.async_receive_from(boost::asio::buffer(*pBuffer), pConnection->mRemoteEndpoint, 
		boost::bind(asyncReadHandler, pConnection, pBuffer, boost::asio::placeholders::bytes_transferred
		, boost::asio::placeholders::error));
	
	return pConnection;
}

