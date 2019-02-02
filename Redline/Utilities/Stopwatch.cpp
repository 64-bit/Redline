#include "stdafx.h"
#include "Stopwatch.h"

Redline::Stopwatch::Stopwatch()
{
	_start = std::chrono::high_resolution_clock::now();
}

void Redline::Stopwatch::Stop()
{
	if(!_hasStopped)
	{
		_end = std::chrono::high_resolution_clock::now();
		_hasStopped = true;
	}
}

float Redline::Stopwatch::GetSeconds()
{
	return GetMiliseconds() / 1000.0f;
}

const double NANOSECONDS_TO_MILISECONDS = 1000000.0;

float Redline::Stopwatch::GetMiliseconds()
{
	Stop();

	auto nanoSeconds = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(_end - _start).count();
	auto miliseconds = (float)(nanoSeconds / NANOSECONDS_TO_MILISECONDS);
	return miliseconds;
}
