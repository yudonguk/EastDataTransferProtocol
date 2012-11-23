#pragma once

#include <boost/noncopyable.hpp>

#include "East/IO/IoConnectionHandler.h"
#include "East/IO/IoConnection.h"

class IoConnector : private boost::noncopyable
{
public:
	virtual ~IoConnector() {}

public:
	virtual IoConnection* Connect(IoConnectionHandler* ioConnectionHandler) = 0;
};
