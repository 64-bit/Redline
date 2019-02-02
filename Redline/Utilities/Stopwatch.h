#pragma once
#include <chrono>

namespace Redline
{
	class Stopwatch
	{
	public:
		Stopwatch();

		void Stop();

		float GetSeconds();
		float GetMiliseconds();

	private:
		bool _hasStopped = false;
		std::chrono::time_point<std::chrono::high_resolution_clock> _start;
		std::chrono::time_point<std::chrono::high_resolution_clock> _end;
	};
}