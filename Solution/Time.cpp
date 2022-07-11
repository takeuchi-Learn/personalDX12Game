#include "Time.h"

using namespace std::chrono;

Time::~Time() {
}

Time::Time() :
	startTimeDir(steady_clock::now()){
}

Time::timeType Time::getNowTime() {
	return duration_cast<timeUnit>(steady_clock::now() - startTimeDir).count();
}

void Time::reset() { startTimeDir = steady_clock::now(); }