#include "stdafx.h"
#include "ThreadpoolThread.h"
#include "IThreadpoolJob.h"
#include "ThreadLocalRandom.h"

using namespace Redline;
using namespace std;

ThreadpoolThread::ThreadpoolThread(Threadpool& owner, EThreadPriority threadPriority, int randomSeed)
	: _threadpool(owner)
{
	_randomSeed = randomSeed;
	_currentJob = nullptr;
	_isRunning = true;
	//Start thread
	_thread = new thread(std::bind(&ThreadpoolThread::ThreadRunLoop, this));
	//TODO: Assign priority
}

//TODO:Delete thread

ThreadpoolThread::~ThreadpoolThread()
{
	_isRunning = false;
	_waitForWorkGate.OpenOne();
	_thread->join();
}

void ThreadpoolThread::AssignWork(shared_ptr<IThreadpoolJob>& job)
{//TODO:Protect this
	_currentJob = job;
	_waitForWorkGate.OpenOne();
}

void ThreadpoolThread::StopThread()
{
	_isRunning = false;
	_waitForWorkGate.OpenOne();
}

void ThreadpoolThread::ThreadRunLoop()
{
	ThreadLocalRandom::InitForThread(_randomSeed);
	while(_isRunning)
	{
		if(_currentJob == nullptr)
		{
			//wait for work and then continue
			_waitForWorkGate.AwaitOpen();
			continue;
		}
		//Else execute job and then try to get a new one
		_currentJob->Execute();
		_currentJob = nullptr;

		if(!_threadpool.TryGetWork(_currentJob))
		{
			_threadpool.ReturnSelfToIdle(this);
		}
	}
}