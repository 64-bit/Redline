#pragma once
#include <mutex>
namespace Redline
{
	//A more advanced gate that automaticly closes and lets only a single thread though at a time
	//Threads call AwaitOpen to wait until this gate is opened
	//When another thread calls OpenOne, one of the waiting threads will proceed.
	//If this is set to open , it will remain open untill one thread passes though, automaticly closing the gate
	class AutoResetThreadGate
	{
	public:
		AutoResetThreadGate(bool isOpen = false);

		bool GetIsOpen();

		void OpenOne();

		void Close();

		void AwaitOpen();

	private:
		bool _isOpen;
		std::mutex _mutex;
		std::condition_variable _condition;
	};
}