#pragma once
#include <mutex>
namespace Redline
{
	//The most basic thread gate class,
	//Threads call AwaitOpen to wait until this gate is open,
	//When another thread calls Open, the waiting threads will proceed
	class ThreadGate
	{
	public:
		ThreadGate(bool isOpen = false);

		bool GetIsOpen();

		void Open();

		void Close();

		void AwaitOpen();

	private:
		bool _isOpen;
		std::mutex _mutex;
		std::condition_variable _condition;
	};
}