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

	/// @brief 一拍の時間を取得
	/// @param bpm 一分間の拍数
	/// @return 一拍の時間
	static inline timeType getOneBeatTime(const float bpm)
	{
		return timeType(std::chrono::duration_cast<timeUnit>(std::chrono::seconds(60ll)).count() / bpm);
	};

	/// @brief エポックからの経過時間を取得
	/// @return 経過時間
	static inline timeType getNowTimeSinceEpoch()
	{
		return std::chrono::duration_cast<timeUnit>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	~Timer();

	Timer();

	/// @brief 起点時間から現在までの時間を取得
	/// @return 起点時間から現在までの時間
	inline timeType getNowTime() const
	{
		return std::chrono::duration_cast<timeUnit>(std::chrono::steady_clock::now() - startTimeDir).count();
	}

	/// @brief 現在を起点時間とする(現在を0とする)
	inline void reset() { startTimeDir = std::chrono::steady_clock::now(); }
};
