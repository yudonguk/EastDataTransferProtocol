#pragma once

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/detail/endpoint.hpp>
#include <boost/asio/buffer.hpp>

class IoSocket : boost::noncopyable
{
public:
	friend class IoConnector;
	
public:
	virtual ~IoSocket(){}

public:
	virtual void AsyncRead() = 0;
	virtual void AsyncWrite(const boost::asio::const_buffer& input) = 0;

	virtual size_t Read(boost::asio::mutable_buffer& output) = 0;
	virtual size_t Write(const boost::asio::const_buffer& input) = 0;

	virtual bool Close() = 0;
};
