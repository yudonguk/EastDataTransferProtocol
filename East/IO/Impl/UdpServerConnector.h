#pragma once

#include "East/IO/IoConnector.h"
#include "East/IO/IoServiceManager.h"

class UdpServerConnector : public IoConnector
{
public:
	UdpServerConnector(IoServiceManager& ioServiceManager, uint16_t port);
	virtual ~UdpServerConnector();

public:
	virtual IoConnection* Connect(IoConnectionHandler* ioConnectionHandler);

private:
	IoServiceManager& mIoServiceManager;
	uint16_t mPort;
};
