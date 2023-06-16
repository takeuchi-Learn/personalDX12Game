/**
 * @file Timer.cpp
 * @brief 時間取得クラス
 */

#include "Timer.h"

using namespace std::chrono;

Timer::~Timer()
{}

Timer::Timer() :
	startTimeDir(steady_clock::now())
{}