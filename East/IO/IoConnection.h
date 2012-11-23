#pragma once

#include <boost/noncopyable.hpp>

#include "East/IO/IoConnectionHandler.h"

class IoConnection : private boost::noncopyable
{
public:
	virtual ~IoConnection(){}

public:
	virtual void RegisterConnectionHandler(IoConnectionHandler* pConnectionHandler) = 0;
	virtual IoConnectionHandler* UnregisterConnectionHandler() = 0;
	virtual bool Close() = 0;
};
