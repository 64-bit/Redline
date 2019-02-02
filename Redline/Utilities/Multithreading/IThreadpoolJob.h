#pragma once

namespace Redline
{
	//The most base featureless class of threadpool job,
	//All that it will do is have execute called on a thread
	class IThreadpoolJob
	{
	public:

		virtual void Execute() = 0;

	private:
	};
}