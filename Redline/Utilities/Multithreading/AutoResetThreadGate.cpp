#include "stdafx.h"
#include "AutoResetThreadGate.h"

using namespace Redline;
using namespace std;

Redline::AutoResetThreadGate::AutoResetThreadGate(bool isOpen)
{
	_isOpen = isOpen;
}

bool AutoResetThreadGate::GetIsOpen()
{
	std::unique_lock<mutex> lock(_mutex);
	return _isOpen;
}

void AutoResetThreadGate::OpenOne()
{
	std::unique_lock<mutex> lock(_mutex);
	_isOpen = true;
	_condition.notify_one();
}

void AutoResetThreadGate::Close()
{
	std::unique_lock<mutex> lock(_mutex);
	_isOpen = false;
}

void AutoResetThreadGate::AwaitOpen()
{
	std::unique_lock<mutex> lock(_mutex);
	while (!_isOpen)
	{
		_condition.wait(lock);
	}
	_isOpen = false;
}
