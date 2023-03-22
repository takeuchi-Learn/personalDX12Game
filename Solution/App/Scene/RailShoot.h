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
#include "GameObject/Player/Player.h"
#include <GameObject/Player/Reticle.h>
#include <Sound/Sound.h>

#include <Util/Util.h>

/// @brief レールシューティングのシーン
class RailShoot
	: public GameScene
{
	DX12Base* dxBase = nullptr;
	Input* input = nullptr;

	std::unique_ptr<CameraObj> camera;

	std::unique_ptr<Light> light;

	std::unique_ptr<Timer> timer;


#pragma region ImGui

	static constexpr ImGuiWindowFlags winFlags =
		// リサイズ不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		// タイトルバー無し
		ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
		// 設定を.iniに出力しない
		ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings |
		// 移動不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;
	//// スクロールバーを常に表示
	//ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysHorizontalScrollbar |
	//ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar;

	// 最初のウインドウの位置を指定
	static constexpr DirectX::XMFLOAT2 fstWinPos =
		DirectX::XMFLOAT2((float)WinAPI::window_width * 0.02f,
						  (float)WinAPI::window_height * 0.02f);

	static constexpr inline float playerHpBarWidMax = (float)WinAPI::window_width / 4.f;
	float playerHpBarNowRaito = 0.f;
	float playerFrontHpBarNowRaito = 0.f;

#pragma endregion

#pragma region 音

	std::unique_ptr<Sound> killSe;
	std::unique_ptr<Sound> bgm;

#pragma endregion 音

	// --------------------
	// スプライト
	// --------------------
	std::unique_ptr<SpriteBase> spriteBase;
	std::unique_ptr<DebugText> debugText;

	const UINT aimGrNum;
	std::unique_ptr<Sprite> cursorGr;

	std::forward_list<Reticle> reticle;

	// 操作説明
	std::unordered_map<std::string, std::unique_ptr<Sprite>> operInst;
	const float operInstPosR;

	// --------------------
	// 3Dオブジェクト
	// --------------------
	// 背景のパイプライン
	size_t backPipelineSet;

	// 背景と地面
	std::unique_ptr<Object3d> backObj;
	std::unique_ptr<ObjModel> backModel;
	std::unique_ptr<Object3d> groundObj;
	std::unique_ptr<ObjModel> groundModel;

	// 敵
	std::forward_list<std::shared_ptr<NormalEnemy>> enemy;
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
	std::unique_ptr<ObjModel> ringModel;
	std::vector<std::vector<std::unique_ptr<Object3d>>> laneWall;

	// --------------------
	// 敵発生関連
	// --------------------

	void loadEnemyScript();

	struct PopEnemyData
	{
		uint16_t popFrame;
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 vel{ 0,0,-1 };
		PopEnemyData(uint16_t popFrame,
					 const DirectX::XMFLOAT3& pos,
					 const DirectX::XMFLOAT3& vel)
			: popFrame(popFrame), pos(pos), vel(vel)
		{}
	};

	std::forward_list<std::unique_ptr<PopEnemyData>> enemyPopData;
	uint16_t nowFrame = 0u;

	void startRgbShift();
	void updateRgbShift();

	/// @brief 天球を回す
	void rotationBack();

	/// @brief 敵を追加
	/// @param pos 敵の位置
	/// @param vel 敵の速度
	/// @param scale 敵の大きさ
	void addEnemy(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& vel, float scale = 5.f);

	template<class NextScene>
	void changeNextScene();

	// update_何とか関数を格納する
	std::function<void()> update_proc;
	void update_start();
	void update_appearPlayer();
	void update_play();
	void update_exitPlayer();
	template<class NextScene>
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
		startEnd<DirectX::XMFLOAT3> playerScale;
		startEnd<float> camFogRad;
	};

	inline static constexpr float camFogStart = DirectX::XM_PI / 9.f;
	inline static constexpr float camFogEnd = DirectX::XM_PI / 3.f;

	inline const static DirectX::XMFLOAT3 appearPPosEnd = DirectX::XMFLOAT3(0, 12, 0);
	inline const static DirectX::XMFLOAT3 appearPPosStart = DirectX::XMFLOAT3(appearPPosEnd.x,
																			  appearPPosEnd.y,
																			  appearPPosEnd.z - 1000.f);

	std::unique_ptr<AppearPlayer> appearPlayer;

	void initFixedCam(const DirectX::XMFLOAT3& startPos,
					  const DirectX::XMFLOAT3& endPos);

	void startAppearPlayer();
	void endAppearPlayer();

#pragma endregion 自機登場演出

#pragma region 自機退場演出

	struct ExitPlayer
	{
		startEnd<DirectX::XMFLOAT3> playerPos;
		const Timer::timeType exitTime;
		std::unique_ptr<Timer> timer;
		startEnd<DirectX::XMFLOAT3> playerScale;
	};
	std::unique_ptr<ExitPlayer> exitPlayer;

	void startExitPlayer();
	void endExitPlayer();

#pragma endregion 自機退場演出

	void updateRailPos();
	void movePlayer();
	void updatePlayerShotTarget(const DirectX::XMFLOAT2& aim2DMin, const DirectX::XMFLOAT2& aim2DMax);

public:
	RailShoot();

	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	~RailShoot();
};
