﻿#include "Util.h"
#include <fstream>
#include <sstream>

#define INI_STRNICMP(s1, s2, cnt) ( _strnicmp( s1, s2, cnt ) )
#define INI_IMPLEMENTATION
#include <ExternalCode/ini.h>

std::unordered_map<std::string, Util::CSVType> Util::loadedCsvData{};

using namespace DirectX;

Util::IniData::IniData(const std::string& data) :
	ini(ini_load(data.c_str(), nullptr))
{}
Util::IniData::~IniData()
{
	ini_destroy(ini);
}

const Util::CSVType& Util::loadCsv(const std::string& csvFilePath,
								   bool commentFlag,
								   char divChar,
								   const std::string& commentStartStr)
{
	auto data = loadedCsvData.try_emplace(csvFilePath, CSVType());
	if (!data.second)
	{
		return data.first->second;
	}

	CSVType& csvData = data.first->second;


	std::ifstream ifs(csvFilePath);
	if (!ifs)
	{
		return csvData;
	}

	std::string line{};
	// 開いたファイルを一行読み込む(カーソルも動く)
	while (std::getline(ifs, line))
	{
		// コメントが有効かつ行頭が//なら、その行は無視する
		if (commentFlag && line.find(commentStartStr) == 0U)
		{
			continue;
		}

		// 行数を増やす
		csvData.emplace_back();

		std::istringstream stream(line);
		std::string field;
		// 読み込んだ行を','区切りで分割
		while (std::getline(stream, field, divChar))
		{
			csvData.back().emplace_back(field);
		}
	}

	return csvData;
}

DirectX::XMVECTOR Util::splinePosition(const std::vector<DirectX::XMVECTOR>& points, const size_t& startIndex, float t)
{
	if (startIndex < 1) { return points[1]; }

	{
		size_t n = points.size() - 2;
		if (startIndex > n) { return points[n]; }
	}

	XMVECTOR p0 = points[startIndex - 1];
	XMVECTOR p1 = points[startIndex];
	XMVECTOR p2 = points[startIndex + 1];
	XMVECTOR p3 = points[startIndex + 2];

	XMVECTOR position =
	{
		2 * p1 + (-p0 + p2) * t +
		(2 * p0 - 5 * p1 + 4 * p2 - p3) * (t * t) +
		(-p0 + 3 * p1 - 3 * p2 + p3) * (t * t * t)
	};

	return position * 0.5f;
}