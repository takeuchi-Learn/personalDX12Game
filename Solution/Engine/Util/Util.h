/*****************************************************************//**
 * \file   Util.h
 * \brief  汎用的な機能を入れるクラス
 *********************************************************************/

#pragma once
#include <vector>
#include <string>

/// @brief 汎用的な機能を入れるユーティリティクラス
class Util
{
public:
	/// @brief std::stringの2次元配列(vector)
	using CSVType = std::vector<std::vector<std::string>>;

	// @brief loadCsvの入力をstd::stringにしたもの
	// @return 読み込んだcsvの中身。失敗したらデフォルトコンストラクタで初期化された空のvector2次元配列が返る
	// @param commentFlag //で始まる行を無視するかどうか(trueで無視)
	// @param divChar フィールドの区切り文字
	// @param commentStartStr コメント開始文字
	static CSVType loadCsv(const std::string& csvFilePath,
						   bool commentFlag = true,
						   char divChar = ',',
						   const std::string& commentStartStr = "//");
};
