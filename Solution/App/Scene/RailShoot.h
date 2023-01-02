#pragma once
#include "System/GameScene.h"

#include <memory>
#include <functional>
#include <unordered_map>

#include "2D/DebugText.h"
#include "GameObject/NormalEnemy.h"
#include "3D/ParticleMgr.h"
#include "Input/Input.h"
#include "System/DX12Base.h"
#include "Camera/CameraObj.h"
#include "GameObject/Player.h"
#include "3D/Obj/ObjSet.h"

/// @brief レールシューティングのシーン
class RailShoot
	: public GameScene
{
	DX12Base* dxBase = nullptr;
	Input* input = nullptr;

	std::unique_ptr<CameraObj> camera;

	std::unique_ptr<Light> light;

	std::unique_ptr<Timer> timer;

	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<DebugText> debugText;

	std::unique_ptr<Sprite> aim2D;
	std::unique_ptr<Sprite> hpBar;
	std::unique_ptr<Sprite> hpBarEdge;
	const float hpBarWidMax;

	// 操作説明
	std::unordered_map<std::string, std::unique_ptr<Sprite>> operInst;
	const float operInstPosR;

	// --------------------
	// 3Dオブジェクト
	// --------------------
	// 背景のパイプライン
	Object3d::PipelineSet backPipelineSet;

	// 背景と地面
	std::unique_ptr<ObjSet> back;
	std::unique_ptr<ObjSet> ground;

	// 敵
	std::forward_list<std::unique_ptr<NormalEnemy>> enemy;
	std::unique_ptr<ObjModel> enemyModel;
	std::unique_ptr<ObjModel> enemyBulModel;

	// 自機
	std::unique_ptr<Player> player;
	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<ObjModel> playerBulModel;
	uint16_t playerHpMax;

	// レールの現在位置を示すオブジェクト
	std::unique_ptr<GameObj> railObj;

	// --------------------
	// パーティクル
	// --------------------
	std::unique_ptr<ParticleMgr> particleMgr;

	// --------------------
	// RGBずらし
	// --------------------
	static const Timer::timeType rgbShiftTimeMax = Timer::oneSec / 2;
	Timer::timeType startRgbShiftTime = 0;
	bool rgbShiftFlag = false;

	// --------------------
	// シーン遷移
	// --------------------
	static const Timer::timeType sceneChangeTime = Timer::oneSec;

	Timer::timeType startSceneChangeTime{};
	// --------------------
	// スプライン補間
	// --------------------
	std::vector<DirectX::XMVECTOR> splinePoint;
	uint16_t splineNowFrame = 0u;
	static const uint16_t splineFrameMax = 120u;
	static const uint16_t splineIndexDef = 1u;
	uint16_t splineIndex = splineIndexDef;

	std::unique_ptr<ObjModel> wallModel;
	std::vector <std::pair<std::unique_ptr<Object3d>, std::unique_ptr<Object3d>>> laneWall;

	// --------------------
	// 敵発生関連
	// --------------------

	// std::stringの2次元配列(vector)
	using CSVType = std::vector<std::vector<std::string>>;

	// 敵発生スクリプトのCSVデータ
	CSVType csvData;

	// @brief loadCsvの入力をstd::stringにしたもの
	// @return 読み込んだcsvの中身。失敗したらデフォルトコンストラクタで初期化された空のvector2次元配列が返る
	// @param commentFlag //で始まる行を無視するかどうか(trueで無視)
	// @param divChar フィールドの区切り文字
	// @param commentStartStr コメント開始文字
	CSVType loadCsv(const std::string& csvFilePath,
					bool commentFlag = true,
					char divChar = ',',
					const std::string& commentStartStr = "//");

	struct PopEnemyData
	{
		uint16_t popFrame;
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel{ 0,0,-1 };
		PopEnemyData(uint16_t popFrame,
					 const DirectX::XMFLOAT3& pos,
					 const DirectX::XMFLOAT3& vel)
			: popFrame(popFrame), pos(pos), vel(vel)
		{
		}
	};

	std::forward_list<std::unique_ptr<PopEnemyData>> enemyPopData;
	uint16_t nowFrame = 0u;

	void createParticle(const DirectX::XMFLOAT3& pos,
						const uint16_t particleNum = 10U,
						const float startScale = 1.f,
						const float vel = 5.f);

	void startRgbShift();
	void updateRgbShift();

	/// @brief 天球を回す
	void rotationBack();

	/// @brief 敵を追加
	/// @param pos 敵の位置
	/// @param vel 敵の速度
	/// @param scale 敵の大きさ
	void addEnemy(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& vel, float scale = 5.f);

	void changeNextScene();

	// update_何とか関数を格納する
	std::function<void()> update_proc;
	void update_start();
	void update_appearPlayer();
	void update_play();
	void update_end();


#pragma region 自機登場演出

	template<class T>
	struct startEnd
	{
		T start;
		T end;
	};

	struct AppearPlayer
	{
		startEnd<DirectX::XMFLOAT3> playerPos;
		const Timer::timeType appearTime;
		std::unique_ptr<Timer> timer;
	};

	std::unique_ptr<AppearPlayer> appearPlayer;

	void startAppearPlayer();
	void endAppearPlayer();

#pragma endregion 自機登場演出

	void updateRailPos();
	void movePlayer();
	void updatePlayerShotTarget();

	void updateAimCol();

public:
	RailShoot();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	~RailShoot();

	static DirectX::XMVECTOR splinePosition(const std::vector<DirectX::XMVECTOR>& points,
											const size_t& startIndex,
											float t);
};
