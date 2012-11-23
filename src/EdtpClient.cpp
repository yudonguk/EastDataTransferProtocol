#include "East/Protocol/EDTP/EdtpClient.h"

#include <boost/property_tree/json_parser.hpp>

#include "East/Protocol/EDTP/Service.h"
#include "East/IO/Impl/TcpClientConnector.h"
#include "East/IO/IoServiceManager.h"
#include "East/IO/IoConnection.h"
#include "East/Util/JsonHelper.h"

EdtpClient::EdtpClient(const std::string& id, const std::string& serverIp, uint16_t port)
	: mId(id), mpIoServiceManager(new IoServiceManager)
	, mpServerConnector(new TcpClientConnector(*mpIoServiceManager, serverIp, port))
	, mpControlConnection(nullptr)
{
	mpIoServiceManager->Enable();
}

EdtpClient::~EdtpClient()
{
	mpIoServiceManager->Disable();
	delete mpServerConnector;
	delete mpIoServiceManager;
}

bool EdtpClient::RegisterService(boost::shared_ptr<Service>& pService)
{
	auto itor = mMapService.find(pService->GetId());

	if (itor != mMapService.end())
	{
		return false;
	}
	
	mMapService.insert(ServiceMap::_Val_type(pService->GetId(), pService));
	
	return true;
}

bool EdtpClient::UnregisterService(boost::shared_ptr<Service>& pService)
{
	auto itor = mMapService.find(pService->GetId());

	if (itor != mMapService.end())
	{
		return false;
	}

	mMapService.erase(itor);

	return true;
}

bool EdtpClient::Enable()
{
	if (mpControlConnection != nullptr)
	{
		return true;
	}

	mpControlConnection = mpServerConnector->Connect(this);
	
	return true;
}

bool EdtpClient::Disable()
{
	if (mpControlConnection == nullptr)
	{
		return true;
	}

	return mpControlConnection->Close();
}

void EdtpClient::OnConnected(IoSocket& socket)
{
	std::stringstream clientInfoStream;

	boost::property_tree::ptree clientRegisterProperty;
	JsonHelper::Put(clientRegisterProperty, "ClientId", mId);
	JsonHelper::Put(clientRegisterProperty, "ClientPassword", "");

	boost::property_tree::ptree serviceRegisterArray;
	boost::property_tree::ptree serviceRegister;
	for (auto itor = mMapService.begin(); itor != mMapService.end(); ++itor)
	{
		auto& pService = itor->second;		
			
		JsonHelper::Put(serviceRegister, "ServiceId", pService->GetId());
		JsonHelper::Put(serviceRegister, "ServiceType", pService->GetServiceType());

		JsonHelper::Put(serviceRegister, "Bool\"Test", true);
		JsonHelper::Put(serviceRegister, "IntTest", 12345);
		JsonHelper::Put(serviceRegister, "LongTest", 12345l);
		JsonHelper::Put(serviceRegister, "FloatTest", 12.5f);
		JsonHelper::Put(serviceRegister, "DoubleTest", 12.5);
		
		serviceRegisterArray.push_back(std::make_pair("", serviceRegister));
	}
	clientRegisterProperty.put_child("Service", serviceRegisterArray);

	JsonHelper::write_json(clientInfoStream, clientRegisterProperty, false);

	socket.AsyncWrite(boost::asio::buffer(clientInfoStream.str()));
}

void EdtpClient::OnDisconnected(IoSocket& socket)
{
	delete mpControlConnection;
	mpControlConnection = nullptr;
}

void EdtpClient::OnRecevied(IoSocket& socket, boost::asio::const_buffer& data)
{
	
}

void EdtpClient::OnError(IoSocket& socket, const boost::system::error_code& error)
{
	socket.Close();
}
