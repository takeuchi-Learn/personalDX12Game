/*****************************************************************//**
 * \file   Util.h
 * \brief  汎用的な機能を入れるクラス
 *********************************************************************/

#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>

 /// @brief 汎用的な機能を入れるユーティリティクラス
class Util
{
public:
	/// @brief std::stringの2次元配列(vector)
	using CSVType = std::vector<std::vector<std::string>>;

	/// @brief loadCsvの入力をstd::stringにしたもの
	/// @param csvFilePath 読み込むCSVファイルのパス
	/// @param commentFlag //で始まる行を無視するかどうか(trueで無視)
	/// @param divChar フィールドの区切り文字
	/// @param commentStartStr コメント開始文字列
	/// @return 読み込んだcsvの中身。失敗したらデフォルトコンストラクタで初期化された空のvector2次元配列が返る
	static CSVType loadCsv(const std::string& csvFilePath,
						   bool commentFlag = true,
						   char divChar = ',',
						   const std::string& commentStartStr = "//");

	/// @brief 制御点からスプライン補間をする関数
	/// @param points 制御点。始点と終点は同じものを二つ入れる。
	/// @param startIndex 何個目の制御点まで来たか
	/// @param t 二つの制御点間での進行度[0~1]
	/// @return スプライン補間された点の座標
	static DirectX::XMVECTOR splinePosition(const std::vector<DirectX::XMVECTOR>& points,
											const size_t& startIndex,
											float t);
};
