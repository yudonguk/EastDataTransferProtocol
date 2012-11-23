#pragma once

#include <typeinfo>

#include <boost/any.hpp>

class Service
{
public:
	virtual ~Service(){}

public:
	virtual std::string GetId() = 0;
	virtual std::string GetServiceType() = 0;  
};

template<typename ServiceType>
class ServiceImp : public Service
{
public:
	

public:
	std::string GetId()
	{
		return std::string(typeid(ServiceType).name()).append("_Service");
	}

	std::string GetServiceType()
	{
		return typeid(ServiceType).name();
	}
};