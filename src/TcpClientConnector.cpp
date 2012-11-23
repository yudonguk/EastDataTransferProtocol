#include "East/IO/Impl/TcpClientConnector.h"

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>

#include "East/IO/Impl/TcpConnection.h"

TcpClientConnector::TcpClientConnector(IoServiceManager& ioServiceManager, const std::string& serverAddress, uint16_t serverPort)
	: mIoServiceManager(ioServiceManager), mServerAddress(serverAddress), mServerPort(serverPort)
{
	/*
	static const std::string splitter("://");

	auto splitterIndex = serverAddress.find(splitter);
	if (splitterIndex == std::string::npos)
	{
		mServerService = "";
		mServerAddress = serverAddress;
	}
	else
	{
		mServerService = serverAddress.substr(0, splitterIndex);
		mServerAddress = serverAddress.substr(splitterIndex + splitter.size());
	}		

	boost::algorithm::trim(mServerService);
	*/
	boost::algorithm::trim(mServerAddress);
}

TcpClientConnector::~TcpClientConnector()
{}

IoConnection* TcpClientConnector::Connect(IoConnectionHandler* ioConnectionHandler)
{
	TcpConnection* pConnection = new TcpConnection(mIoServiceManager.GetIoService(), ioConnectionHandler);

	using namespace boost::asio::ip;

	boost::system::error_code error;
	tcp::resolver resolver(mIoServiceManager.GetIoService());
	tcp::resolver::query query(mServerAddress, "");
	auto endpointItor = resolver.resolve(query, error);
	tcp::endpoint remoteEndpoint;

	if (error || endpointItor == tcp::resolver::iterator())
	{
		delete pConnection;
		pConnection = nullptr;
		return nullptr;
	}

	remoteEndpoint = *endpointItor;
	remoteEndpoint.port(mServerPort);		
	
	static boost::function<void(TcpConnection*, const boost::system::error_code&)> connectHandler =
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
			std::cout << error.message() << std::endl;

			pConnection->mState = TcpConnection::DISCONNECTED;
			if (pConnectionHandler != nullptr)
			{
				pConnectionHandler->OnDisconnected(*pConnection);
			}
		}
	};

	auto& socket = pConnection->mSocket;

	socket.async_connect(remoteEndpoint, boost::bind(connectHandler, pConnection, boost::asio::placeholders::error));
	
	return pConnection;
}
