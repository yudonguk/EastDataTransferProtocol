#pragma once

#include <list>

#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/shared_mutex.hpp>

class Blocker
	: private boost::noncopyable
{
private:
	enum State{NONE_BLOCKING, BLOCKING};

public:
	Blocker()
		: blockedThreadCount(0), state(NONE_BLOCKING)
	{}

	~Blocker()
	{
		Unblock();
	}

public:
	void Block()
	{
		boost::mutex::scoped_lock lock(mutex);
		state = BLOCKING;
	}

	void Unblock()
	{
		boost::mutex::scoped_lock lock(mutex);
		state = NONE_BLOCKING;
		condition.notify_all();
	}

	inline size_t BlockedCount()
	{
		boost::mutex::scoped_lock lock(mutex);
		return blockedThreadCount;
	}

	inline void Wait()
	{
		boost::mutex::scoped_lock lock(mutex);

		blockedThreadCount++;

		while (state == BLOCKING)
			condition.wait(lock);

		blockedThreadCount--;
	}

private:
	size_t blockedThreadCount;
	State state;
	boost::mutex mutex;
	boost::condition_variable condition;
};

class IoServiceManager : 
	private boost::noncopyable
{
public:
	enum ThreadGroupState{};
	
public:
	IoServiceManager(size_t threadCount = 4);
	~IoServiceManager();

public:
	bool Enable();
	bool Disable();

	boost::asio::io_service& GetIoService();

private:
	void PorcessIoServiceHandler();

private:
	boost::asio::io_service mIoService;
	boost::asio::io_service::work mWork;
	boost::thread_group mThreadGroup;
	Blocker mBlockerThreadGroup;
};