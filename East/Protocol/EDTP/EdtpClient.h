#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

#include <boost/smart_ptr.hpp>

#include "East/IO/IoConnectionHandler.h"

class Service;
class IoServiceManager;
class IoConnector;
class IoConnection;

class EdtpClient : public IoConnectionHandler
{
public:
	typedef std::unordered_map<std::string, boost::shared_ptr<Service>> ServiceMap;

public:
	EdtpClient(const std::string& id, const std::string& serverIp, uint16_t port);
	~EdtpClient();

public:
	bool Enable();
	bool Disable();

	bool RegisterService(boost::shared_ptr<Service>& pService);
	bool UnregisterService(boost::shared_ptr<Service>& pService);

public:
	void OnConnected(IoSocket& socket);
	void OnDisconnected(IoSocket& socket);
	void OnRecevied(IoSocket& socket, boost::asio::const_buffer& data);
	void OnError(IoSocket& socket, const boost::system::error_code& error);

private:
	std::string mId;
	ServiceMap mMapService;

	IoServiceManager* const mpIoServiceManager;
	IoConnector* const mpServerConnector;

	IoConnection* mpControlConnection;

};

