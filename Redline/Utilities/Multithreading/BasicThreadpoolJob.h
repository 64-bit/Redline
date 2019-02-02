#pragma once

#include "IThreadpoolJob.h"
#include <functional>
#include "Promis.h"

namespace Redline
{
	//A basic threadpool job, that can return a object of type T either in a callback or a promis
	template
	<class T>
	class BasicThreadpoolJob : public IThreadpoolJob
	{
	public:

		BasicThreadpoolJob(std::function<T()>& work);

		BasicThreadpoolJob(std::function<T()>& work, std::function<void(T&)>& callback);

		void Execute() override;

		std::shared_ptr<Promis<T>> GetPromis();
	private:

		void InteralExecute();

		std::function<T()> _work;
		std::function<void(T&)> _callback;
		std::shared_ptr<Promis<T>> _promis;
	};

	template <class T>
	BasicThreadpoolJob<T>::BasicThreadpoolJob(std::function<T()>& work)
	{
		_work = work;
		_callback = nullptr;
		_promis = nullptr;
	}

	template <class T>
	BasicThreadpoolJob<T>::BasicThreadpoolJob(std::function<T()>& work, std::function<void(T&)>& callback)
	{
		_work = work;
		_callback = callback;
		_promis = nullptr;
	}

	template <class T>
	void BasicThreadpoolJob<T>::Execute()
	{
		InteralExecute();
	}

	template <class T>
	std::shared_ptr<Promis<T>> BasicThreadpoolJob<T>::GetPromis()
	{
		if(_promis == nullptr)
		{
			_promis = std::make_shared<Promis<T>>();
		}
		return _promis;
	}

	template<class T>
	void BasicThreadpoolJob<T>::InteralExecute()
	{
		auto result = _work();
		if(_promis != nullptr)
		{
			_promis->Fulfill(result);
		}
		if(_callback != nullptr)
		{
			_callback(result);
		}
	}
}
