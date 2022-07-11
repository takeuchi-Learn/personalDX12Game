#pragma once

#include<random>

class RandomNum {
private:
	RandomNum(const RandomNum& mrnd) = delete;
	RandomNum& operator=(const RandomNum& mrnd) = delete;

	std::random_device rnd{};
	std::mt19937_64 mt{};

	RandomNum() : rnd(), mt(rnd()) {};
	~RandomNum() {};

	static RandomNum* getInstance();

private:
	// 一様乱数
	int local_getRand(const int min, const int max);
	double local_getRand(const double min, const double max);

	// 正規分布乱数
	double local_getRandNormally(const double center, const double range);

public:
	// 一様乱数_整数
	static int getRand(const int min, const int max);
	// 一様乱数_小数(double)
	static double getRand(const double min, const double max);
	// 一様乱数_小数(float)
	static float getRandf(const float min, const float max);

	// 正規分布乱数_double
	static double getRandNormally(const double center, const double range);
	// 正規分布乱数_float
	static float getRandNormallyf(const float center, const float range);
};

