#pragma once

#include <boost/asio.hpp>

#include "East/IO/IoConnector.h"
#include "East/IO/IoServiceManager.h"

class TcpServerConnector : public IoConnector
{
public:
	TcpServerConnector(IoServiceManager& ioServiceManager, uint16_t port);
	virtual ~TcpServerConnector();

public:
	virtual IoConnection* Connect(IoConnectionHandler* ioConnectionHandler);

private:
	IoServiceManager& mIoServiceManager;
	boost::asio::ip::tcp::acceptor mAcceptor;
};

