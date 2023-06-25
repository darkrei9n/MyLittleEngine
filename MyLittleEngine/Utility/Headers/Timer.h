#pragma once

#include "CommonHeader.h"

#if true
class Timer
{
	long long currTime;
	long long baseTime;
	long long stopTime;
	long long prevTime;
	long long pauseTime;

	double deltaTime;
	double secondsPerCount;
	bool stopped;

public:
	Timer();
	void tick();
	double getDelta();
	void reset();
	void pauseTimer();
	void startTimer();
};


#endif