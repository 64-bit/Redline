#pragma once

#include <thread>
#include "Threadpool.h"

namespace Redline
{
	class ThreadpoolThread
	{
	public:

		ThreadpoolThread(Threadpool& owner, EThreadPriority threadPriority, int randomSeed);
		~ThreadpoolThread();
		void AssignWork(std::shared_ptr<IThreadpoolJob>& job);

		void StopThread();

	private:

		void ThreadRunLoop();
		
		bool _isRunning = true;
		int _randomSeed;
		std::thread* _thread;
		Threadpool& _threadpool;
		AutoResetThreadGate _waitForWorkGate;
		std::shared_ptr<IThreadpoolJob> _currentJob;
	};
}
