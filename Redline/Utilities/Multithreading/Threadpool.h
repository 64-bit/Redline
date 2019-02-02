#pragma once

#include "Promis.h"
#include "AutoResetThreadGate.h"
#include <functional>
#include <vector>
#include <stack>
#include <queue>

namespace Redline
{
	enum EThreadPriority
	{
		Lowest,
		Low,
		Normal,
		High,
		Highest
	};

	class ThreadpoolThread;
	class IThreadpoolJob;

	//A threadpool that can run either std::function<void()> methods, or threadpool jobs
	//that support more advanced scheduling features
	class Threadpool
	{
		friend ThreadpoolThread;
	public:
		//Create a threadpool with one thread per core, and default thread priority
		Threadpool();
		//Create a threadpool with a specified number of threads and priority
		Threadpool(unsigned threadCount, EThreadPriority priority);

		~Threadpool();

		//Runs a job and creates a basic threadpool job to contain it
		void RunJob(std::function<void()>& job);

		//Runs a job, returns nothing as the job itself should take care of it's promis
		void RunJob(std::shared_ptr<IThreadpoolJob>& job);

		int GetThreadCount();

	private:
		//Called by threads when they finish a task
		bool TryGetWork(std::shared_ptr<IThreadpoolJob>& jobOut);

		void ReturnSelfToIdle(ThreadpoolThread* thread);

		std::vector<ThreadpoolThread*> _threads;

		std::stack<ThreadpoolThread*> _idleThreads;

		std::queue<std::shared_ptr<IThreadpoolJob>> _pendingWork;

		std::mutex _workControlMutex;
	};
}
