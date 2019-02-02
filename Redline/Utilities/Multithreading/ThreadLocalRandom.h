#pragma once
#include <random>

namespace Redline
{
	class ThreadLocalRandom
	{
	public:

		static void InitForThread(int seed);

		static int IntRange(int min, int max);

		static float FloatRange(float min, float max);

		static float NormalFloat();
	private:
		static thread_local std::mt19937 _generator;
	};
}
