#include "Headers/Timer.h"

Timer::Timer() : secondsPerCount(0.0), deltaTime(-1.0), baseTime(0), pauseTime(0), prevTime(0), currTime(0), stopped(false)
{
	long long secondsCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&secondsCount);
	secondsPerCount = 1 / (double)secondsCount;
}

void Timer::tick()
{
	if (stopped)
	{
		deltaTime = 0;
		return;
	}
	long long curr;
	QueryPerformanceCounter((LARGE_INTEGER*)&curr);
	currTime = curr;

	deltaTime = (curr - prevTime) * secondsPerCount;

	prevTime = currTime;

	if (deltaTime < 0.0)
	{
		deltaTime = 0.0;
	}
}

double Timer::getDelta()
{
	return deltaTime;
}

void Timer::reset()
{
	long long  curr;
	QueryPerformanceCounter((LARGE_INTEGER*)&curr);
	currTime = curr;
}

void Timer::pauseTimer()
{
	if (!stopped)
	{
		long long curr;
		QueryPerformanceCounter((LARGE_INTEGER*)&curr);

		stopTime = curr;
		stopped = true;
	}

	
}

void Timer::startTimer()
{
	if (stopped)
	{
		
		stopped = false;
	}
	
}