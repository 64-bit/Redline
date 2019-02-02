#include "stdafx.h"
#include "ThreadGate.h"

using namespace Redline;
using namespace std;

ThreadGate::ThreadGate(bool isOpen)
{
	_isOpen = isOpen;
}

bool ThreadGate::GetIsOpen()
{
	std::unique_lock<mutex> lock(_mutex);
	return _isOpen;
}

void ThreadGate::Open()
{
	std::unique_lock<mutex> lock(_mutex);
	_isOpen = true;
	_condition.notify_all();
}

void ThreadGate::Close()
{
	std::unique_lock<mutex> lock(_mutex);
	_isOpen = false;
}

void Redline::ThreadGate::AwaitOpen()
{
	std::unique_lock<mutex> lock(_mutex);
	while(!_isOpen)
	{
		_condition.wait(lock);
	}
}
