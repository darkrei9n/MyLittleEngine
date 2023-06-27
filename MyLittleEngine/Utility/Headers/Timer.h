#pragma once

#define _int64 long long int

class Timer
{
	_int64 currTime;
	_int64 baseTime;
	_int64 stopTime;
	_int64 prevTime;
	_int64 pauseTime;

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