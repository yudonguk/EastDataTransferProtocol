#pragma once

#include <string>

#include "East/IO/IoConnector.h"
#include "East/IO/IoServiceManager.h"

class TcpClientConnector : public IoConnector
{
public:
	TcpClientConnector(IoServiceManager& ioServiceManager, const std::string& serverAddress, uint16_t serverPort);
	virtual ~TcpClientConnector();

public:
	virtual IoConnection* Connect(IoConnectionHandler* ioConnectionHandler);

private:
	IoServiceManager& mIoServiceManager;
	std::string mServerAddress;
	std::string mServerService;
	uint16_t mServerPort;
};

