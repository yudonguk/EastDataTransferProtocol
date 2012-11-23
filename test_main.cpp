#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "East/IO/IoConnectionHandler.h"
#include "East/IO/IoServiceManager.h"

#include "East/IO/Impl/TcpServerConnector.h"
#include "East/IO/Impl/UdpServerConnector.h"

boost::mutex mutex;

class TestServerHandler : public IoConnectionHandler
{
public:
	TestServerHandler()
		: mIoStream(&mStreamBuffer)
	{
	}
	virtual ~TestServerHandler() {}

public:
	boost::asio::io_service::strand* mStrand;
	boost::asio::streambuf mStreamBuffer;
	std::iostream mIoStream;

	virtual void OnConnected(IoSocket& socket)
	{
		printf("[%d][%08X] OnConnected \r\n", GetTickCount(), this);
		socket.AsyncRead();
	}
	
	virtual void OnDisconnected(IoSocket& socket)
	{
		printf("[%d][%08X] OnDisconnected \r\n", GetTickCount(), this);
		socket.Close();
	}
		
	virtual void OnRecevied(IoSocket& socket, boost::asio::const_buffer& data)
	{
		socket.AsyncRead();
		socket.AsyncWrite(data);
		
		size_t size = boost::asio::buffer_size(data);
		const uint8_t* pData = boost::asio::buffer_cast<const uint8_t*>(data);
		
		mIoStream << std::string(pData, pData + size);
		
		std::string line;
		std::getline(mIoStream, line);
		if (!line.empty())
			std::cout << line << std::endl;

		//printf("[%d][%08X] OnConnected : %s \r\n", GetTickCount(), this, std::string(pData, pData + size).c_str());
	}

	virtual void OnSent(IoSocket& socket)
	{
	}

	virtual void OnError(IoSocket& socket, const boost::system::error_code& error)
	{
		printf("[%d][%08X] OnError : %s \r\n", GetTickCount(), this, error.message().c_str());
		socket.Close();
	}
};

class TestClientHandler : public IoConnectionHandler
{
public:
	TestClientHandler()
	{
	}
	virtual ~TestClientHandler() {}

public:
	boost::asio::io_service::strand* mStrand;

	virtual void OnConnected(IoSocket& socket)
	{
		printf("[%d][%08X] OnConnected \r\n", GetTickCount(), this);
		char* test = "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello";
		socket.AsyncRead();
		socket.AsyncWrite(boost::asio::buffer(test, strlen(test)));
	}
	
	virtual void OnDisconnected(IoSocket& socket)
	{
		printf("[%d][%08X] OnDisconnected \r\n", GetTickCount(), this);
		socket.Close();
	}

	boost::mutex mutex;
	
	virtual void OnRecevied(IoSocket& socket, boost::asio::const_buffer& data)
	{
		socket.AsyncRead();
		socket.AsyncWrite(data);

		size_t size = boost::asio::buffer_size(data);
		const uint8_t* pData = boost::asio::buffer_cast<const uint8_t*>(data);

		//printf("[%d][%08X] OnConnected : %s \r\n", GetTickCount(), this, std::string(pData, pData + size).c_str());
	}

	virtual void OnSent(IoSocket& socket)
	{
	}

	virtual void OnError(IoSocket& socket, const boost::system::error_code& error)
	{
		printf("[%d][%08X] OnError : %s \r\n", GetTickCount(), this, error.message().c_str());
		socket.Close();
	}
};

#include "East/IO/Impl/TcpConnection.h"
#include "East/IO/Impl/TcpClientConnector.h"
#include "East/IO/Impl/UdpConnection.h"
#include "East/IO/Impl/UdpServerConnector.h"
#include "East/Protocol/EDTP/EdtpClient.h"
#include "East/Protocol/EDTP/Service.h"

#include <boost/iostreams/stream.hpp>

int main()
{
	IoServiceManager ioServiceManger;

	ioServiceManger.Enable();

	TcpServerConnector serverConnector(ioServiceManger, 8888);
	//TcpClientConnector clientConnector(ioServiceManger, "127.0.0.1", 8888);
	//EdtpClient client("Test1", "127.0.0.1", 8888);
	//client.RegisterService(boost::shared_ptr<Service>(new ServiceImp<int>));
	//client.RegisterService(boost::shared_ptr<Service>(new ServiceImp<std::string>));
	//client.RegisterService(boost::shared_ptr<Service>(new ServiceImp<long>));
	//client.RegisterService(boost::shared_ptr<Service>(new ServiceImp<float>));

RESTART:	
	serverConnector.Connect(new TestServerHandler);
	//clientConnector.Connect(new TestClientHandler);
	//client.Enable();

	getchar();
	
	goto RESTART;

	ioServiceManger.Disable();

	return 0;
}