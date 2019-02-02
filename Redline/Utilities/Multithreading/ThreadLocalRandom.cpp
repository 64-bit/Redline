#include "stdafx.h"
#include "ThreadLocalRandom.h"

using namespace Redline;

thread_local std::mt19937 ThreadLocalRandom::_generator;

void ThreadLocalRandom::InitForThread(int seed)
{
	_generator.seed(seed);
}

int ThreadLocalRandom::IntRange(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(_generator);
}

float ThreadLocalRandom::FloatRange(float min, float max)
{
	if(min > max)
	{
		float t = max;
		max = min;
		min = t;
	}

	float normal = NormalFloat();
	float dist = max - min;
	return min + normal * dist;
}

float ThreadLocalRandom::NormalFloat()
{
	float val = (float)IntRange(0, 10000000);
	val /= 10000000.0f;
	return val;
}