#pragma once

#include <chrono>

class Timer
{
private:
	std::chrono::steady_clock::time_point  startTimeDir{};

public:
	using timeUnit = std::chrono::microseconds;
	using timeType = long long;

	static constexpr timeType oneSec = std::chrono::duration_cast<timeUnit>(std::chrono::seconds(1)).count();
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
