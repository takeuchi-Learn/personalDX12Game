/**
 * @file RandomNum.h
 * @brief 乱数生成
 */

#pragma once

#include<random>

/// @brief 乱数生成クラス
class RandomNum
{
private:
	RandomNum(const RandomNum& mrnd) = delete;
	RandomNum& operator=(const RandomNum& mrnd) = delete;

	std::random_device rnd{};
	std::mt19937_64 mt{};

	RandomNum() : rnd(), mt(rnd()) {};
	~RandomNum() {};

	static RandomNum* getInstance();

private:
	int local_getRand(const int min, const int max);

	double local_getRand(const double min, const double max);

	double local_getRandNormally(const double center, const double range);

public:
	/// @brief 一様乱数int型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 生成された乱数
	static int getRand(const int min, const int max);

	/// @brief 一様乱数double型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 生成された乱数
	static double getRand(const double min, const double max);

	/// @brief 一様乱数float型。double型のものを型変換しているだけ。
	/// @param min 最小値
	/// @param max 最大値
	/// @return 生成された乱数
	static float getRandf(const float min, const float max);

	/// @brief 正規分布乱数double型
	/// @param center 中央値
	/// @param range 範囲
	/// @return 生成された乱数
	static double getRandNormally(const double center, const double range);

	/// @brief 正規分布乱数float型。double型のものを型変換しているだけ。
	/// @param center 中央値
	/// @param range 範囲
	/// @return 生成された乱数
	static float getRandNormallyf(const float center, const float range);
};
