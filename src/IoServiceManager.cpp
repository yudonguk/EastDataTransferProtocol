#include "East/IO/IoServiceManager.h"

using boost::asio::io_service;

IoServiceManager::IoServiceManager(size_t threadCount /*= 4*/ )
	: mIoService(), mWork(mIoService)
{
	mBlockerThreadGroup.Block();
	
	auto processIoServiceHandler 
		= boost::bind(&IoServiceManager::PorcessIoServiceHandler, this);
	for (size_t i = 0; i < threadCount; i++)
	{
		mThreadGroup.create_thread(processIoServiceHandler);
	}

	while (mBlockerThreadGroup.BlockedCount() != mThreadGroup.size())
		boost::this_thread::yield();
}

IoServiceManager::~IoServiceManager()
{
	Disable();
	mThreadGroup.interrupt_all();
	mThreadGroup.join_all();
}

bool IoServiceManager::Enable()
{
	if (mBlockerThreadGroup.BlockedCount() != mThreadGroup.size())
	{
		return true;
	}

	new(&mWork)boost::asio::io_service::work(mIoService);
	mBlockerThreadGroup.Unblock();

	return true;
}

bool IoServiceManager::Disable()
{
	if (mBlockerThreadGroup.BlockedCount() == mThreadGroup.size())
	{
		return true;
	}

	mBlockerThreadGroup.Block();
	mIoService.stop();
	while (mBlockerThreadGroup.BlockedCount() != mThreadGroup.size())
		boost::this_thread::yield();

	mWork.~work();
	
	return true;
}

boost::asio::io_service& IoServiceManager::GetIoService()
{	
	return mIoService;
}

void IoServiceManager::PorcessIoServiceHandler()
{
	for (;;)
	{		
		mBlockerThreadGroup.Wait();
		mIoService.run();
	}
}

