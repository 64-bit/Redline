#pragma once
#include <mutex>
#include "ThreadGate.h"

namespace Redline
{

	//A promis class that returns the specified type when fufilled
	template
	<class T>
	class Promis
	{
	public:
		Promis();

		void Fulfill(T& value);

		bool IsFufilled();

		bool TryGetValue(T& outValue);

		T Getvalue();

	private:
		T _value;
		bool _isFulfilled;
		std::mutex _mutex;
		ThreadGate _gate;
	};


	template <class T>
	Promis<T>::Promis()
	{
		_isFulfilled = false;
		_value = T();
	}

	template <class T>
	void Promis<T>::Fulfill(T& value)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_value = value;
		_isFulfilled = true;
		_gate.Open();
	}

	template <class T>
	bool Promis<T>::IsFufilled()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		return _isFulfilled;
	}

	template <class T>
	bool Promis<T>::TryGetValue(T& outValue)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if(_isFulfilled)
		{
			outValue = _value;
		}
		return _isFulfilled;
	}

	template <class T>
	T Promis<T>::Getvalue()
	{
		_gate.AwaitOpen();
		return _value;
	}
}
