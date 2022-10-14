#include "Timer.h"

using namespace std::chrono;

Timer::~Timer()
{
}

Timer::Timer() :
	startTimeDir(steady_clock::now())
{
}

Timer::timeType Timer::getNowTime()
{
	return duration_cast<timeUnit>(steady_clock::now() - startTimeDir).count();
}

void Timer::reset() { startTimeDir = steady_clock::now(); }