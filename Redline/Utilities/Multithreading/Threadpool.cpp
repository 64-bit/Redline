#include "stdafx.h"
#include "Threadpool.h"
#include "ThreadpoolThread.h"
#include "IThreadpoolJob.h"
#include "BasicThreadpoolJob.h"
#include "ThreadLocalRandom.h"

using namespace Redline;
using namespace std;

Threadpool::Threadpool()
	: Threadpool(std::thread::hardware_concurrency(),
		EThreadPriority::Low)
{

}

Threadpool::Threadpool(unsigned threadCount, EThreadPriority priority)
{
	if(threadCount == 0)//Autodetect threadcount if set to 0
	{
		threadCount = std::thread::hardware_concurrency();
	}

	for(unsigned i = 0; i < threadCount;i++)
	{
		int threadSeed = ThreadLocalRandom::IntRange(0, INT_MAX);
		_threads.push_back(new ThreadpoolThread(*this, priority, threadSeed));
		_idleThreads.push(_threads[i]);
	}
}

Threadpool::~Threadpool()
{
	for(auto thread : _threads)
	{
		thread->StopThread();
		delete thread;
	}
	_threads.clear();
}

void Threadpool::RunJob(function<void()>& job)
{
	std::function<int()> wrapper = [job]()
	{
		job();
		return 0;
	};

	shared_ptr<IThreadpoolJob> basicJob = std::make_shared<BasicThreadpoolJob<int>>(wrapper);
	RunJob(basicJob);
}

void Threadpool::RunJob(std::shared_ptr<IThreadpoolJob>& job)
{
	lock_guard<mutex> lock(_workControlMutex);

	//If we have idle threads, give one the job
	if(_idleThreads.size() > 0)
	{
		auto thread = _idleThreads.top();
		_idleThreads.pop();
		thread->AssignWork(job);
		return;
	}

	//Else, add it to the pending work pile
	_pendingWork.push(job);
}

int Threadpool::GetThreadCount()
{
	return (int)_threads.size();
}

bool Threadpool::TryGetWork(std::shared_ptr<IThreadpoolJob>& jobOut)
{
	lock_guard<mutex> lock(_workControlMutex);

	if(_pendingWork.size() == 0)
	{
		jobOut = nullptr;
		return false;
	}

	jobOut = _pendingWork.front();
	_pendingWork.pop();

	return true;
}

void Threadpool::ReturnSelfToIdle(ThreadpoolThread * thread)
{
	lock_guard<mutex> lock(_workControlMutex);
	_idleThreads.push(thread);
}
