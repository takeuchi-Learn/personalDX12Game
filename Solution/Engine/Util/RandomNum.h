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

	inline static RandomNum* ins()
	{
		static RandomNum myRand{};
		return &myRand;
	}

public:
	// シード値に使う値を生成
	inline auto randomSeedNum() { return rnd(); }
	// シード値を変更
	inline void resetSeed(auto seed) { mt = std::mt19937_64(seed); }

	/// @brief 整数の一様乱数
	/// @tparam intType 整数型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 乱数
	template<class intType = int>
	inline static intType getRandInt(intType min, intType max)
	{
		intType minLocal = min, maxLocal = max;
		if (max < min)
		{
			minLocal = max;
			maxLocal = min;
		}
		std::uniform_int_distribution<intType> myRand(minLocal, maxLocal);	// 範囲指定の乱数
		return myRand(ins()->mt);
	}

	/// @brief 実数の一様乱数
	/// @tparam intType 実数型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 乱数
	template<class realType = float>
	inline static realType getRandReal(realType min, realType max)
	{
		realType minLocal = min, maxLocal = max;
		if (max < min)
		{
			minLocal = max;
			maxLocal = min;
		}
		std::uniform_real_distribution<realType> myRand(minLocal, maxLocal);	// 範囲指定の乱数
		return myRand(ins()->mt);
	}

	/// @brief 正規分布乱数
	/// @tparam normalType 実数型
	/// @param center 中央値
	/// @param range 範囲
	/// @return 乱数
	template<class normalType = float>
	inline static normalType getRandNormally(normalType center, normalType range)
	{
		normalType rangeLocal = range;
		if (range < normalType(0.0)) rangeLocal = -rangeLocal;
		else if (range == normalType(0.0)) rangeLocal = normalType(1.0);

		std::normal_distribution<normalType> myRand(center, rangeLocal);	// 範囲指定の乱数
		return myRand(ins()->mt);
	}

	/// @brief 一様乱数int型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 生成された乱数
	inline static int getRand(const int min, const int max)
	{
		return getRandInt<int>(min, max);
	}

	/// @brief 一様乱数double型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 生成された乱数
	inline static double getRand(const double min, const double max)
	{
		return getRandReal<double>(min, max);
	}

	/// @brief 一様乱数float型
	/// @param min 最小値
	/// @param max 最大値
	/// @return 生成された乱数
	inline static float getRandf(const float min, const float max)
	{
		return getRandReal<float>(min, max);
	}

	/// @brief 正規分布乱数double型
	/// @param center 中央値
	/// @param range 範囲
	/// @return 生成された乱数
	inline static double getRandNormally(const double center, const double range)
	{
		return getRandNormally<double>(center, range);
	}

	/// @brief 正規分布乱数float型
	/// @param center 中央値
	/// @param range 範囲
	/// @return 生成された乱数
	inline static float getRandNormallyf(const float center, const float range)
	{
		return getRandNormally<float>(center, range);
	}
};
