#include "East/IO/Impl/TcpServerConnector.h"

#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>

#include "East/IO/Impl/TcpConnection.h"

TcpServerConnector::TcpServerConnector(IoServiceManager& ioServiceManager, uint16_t port)
	: mIoServiceManager(ioServiceManager), mAcceptor(ioServiceManager.GetIoService())
{
	using namespace boost::asio::ip;
	mAcceptor.open(tcp::v4());
	mAcceptor.bind(tcp::endpoint(tcp::v4(), port));
	mAcceptor.listen();
}

TcpServerConnector::~TcpServerConnector()
{}

IoConnection* TcpServerConnector::Connect( IoConnectionHandler* ioConnectionHandler )
{
	TcpConnection* pConnection = new TcpConnection(mIoServiceManager.GetIoService(), ioConnectionHandler);
		

	static boost::function<void(TcpConnection*, const boost::system::error_code&)> acceptHandler =
	[](TcpConnection* pConnection, const boost::system::error_code& error)
	{
		boost::shared_lock<boost::shared_mutex> readerLock(pConnection->mSharedLockConnectionHandler);

		IoConnectionHandler* pConnectionHandler = pConnection->mpConnectionHandler;

		if (!error)
		{
			pConnection->mState = TcpConnection::CONNECTED;
			if (pConnectionHandler != nullptr)
			{
				pConnectionHandler->OnConnected(*pConnection);
			}			
		}
		else
		{
			pConnection->mState = TcpConnection::DISCONNECTED;
			if (pConnectionHandler != nullptr)
			{
				pConnectionHandler->OnDisconnected(*pConnection);
			}		
		}
	};

	mAcceptor.async_accept(pConnection->mSocket
		, boost::bind(acceptHandler, pConnection, boost::asio::placeholders::error));
			
	return pConnection;
}
