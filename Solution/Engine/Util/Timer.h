﻿/**
 * @file Timer.h
 * @brief 時間取得クラス
 */

#pragma once

#include <chrono>

/// @brief 時間取得クラス
class Timer
{
private:
	std::chrono::steady_clock::time_point  startTimeDir{};

public:
	/// @brief 時間の単位
	using timeUnit = std::chrono::microseconds;
	/// @brief 時間を格納する型
	using timeType = long long;

	/// @brief 一秒
	static constexpr timeType oneSec = std::chrono::duration_cast<timeUnit>(std::chrono::seconds(1)).count();
	/// @brief 一秒float型
	static constexpr float oneSecF = static_cast<float>(oneSec);

	~Timer();

	Timer();

	/// @brief 一拍の時間を取得
	/// @param bpm 一分間の拍数
	/// @return 一拍の時間
	inline static timeType getOneBeatTime(const float bpm) { return timeType(std::chrono::duration_cast<timeUnit>(std::chrono::seconds(60ll)).count() / bpm); };

	/// @brief reset()最後に実行した時間が起点
	/// @brief 実行していなければクラス生成時の時間が起点
	/// @return 現在までの時間
	timeType getNowTime();

	/// @brief 現在を0とする
	void reset();
};