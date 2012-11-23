#pragma once

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "East/IO/IoSocket.h"

class IoConnectionHandler
{
public:
	virtual ~IoConnectionHandler() {}

public:
	virtual void OnConnected(IoSocket& socket){}
	virtual void OnDisconnected(IoSocket& socket){}
	virtual void OnRecevied(IoSocket& socket, boost::asio::const_buffer& data){}
	virtual void OnSent(IoSocket& socket){}
	virtual void OnError(IoSocket& socket, const boost::system::error_code& error){}
};