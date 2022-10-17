#pragma once
#include "../Engine/System/GameScene.h"

#include <memory>
#include <DirectXMath.h>
#include <vector>
#include <functional>
#include <utility>
#include <memory>

#include "../Engine/Sound/Sound.h"
#include "../Engine/Camera/CameraObj.h"
#include "../Engine/3D/Obj/ObjSet.h"
#include "../Engine/Util/Timer.h"
#include "../Engine/3D/Fbx/FbxObj3d.h"
#include "../Engine/3D/ParticleMgr.h"
#include "../GameObject/Player.h"
#include "../Engine/2D/DebugText.h"
#include "../Engine/Input/Input.h"

class PlayScene :
	public GameScene
{
#pragma region カメラ

	std::unique_ptr<CameraObj> camera;
	DirectX::XMFLOAT2 cameraMoveVel{};

#pragma endregion カメラ

#pragma region 音

	std::unique_ptr<Sound> bgm;

	std::unique_ptr<Sound> particleSE;

#pragma endregion 音

#pragma region スプライト
	// --------------------
	// スプライト共通
	// --------------------
	std::unique_ptr<SpriteBase> spriteBase;
	UINT texNum = 0u;

	// --------------------
	// スプライト個別
	// --------------------

	// --------------------
	// デバッグテキスト
	// --------------------
	std::unique_ptr<DebugText> debugText;

	// デバッグテキスト用のテクスチャ番号
	UINT debugTextTexNumber;

#pragma endregion スプライト

#pragma region ライト

	std::unique_ptr<Light> light;

#pragma endregion ライト

#pragma region 3Dオブジェクト

	// 3Dオブジェクト用パイプライン
	Object3d::PipelineSet backPipelineSet;

	/// @brief 天球
	std::unique_ptr<ObjSet> back;

	/// @brief 地面(背景)
	std::unique_ptr<ObjSet> ground;

	/// @brief ボス
	std::unique_ptr<ObjSet> boss;
	std::unique_ptr<Timer::timeType> bossStartTime;
	bool bossAlive = true;

	/// @brief 自機
	std::unique_ptr<ObjModel> playerModel;
	std::unique_ptr<Player> player;
	std::pair<std::unique_ptr<ObjSet>, bool> playerBul;	// second : 生存フラグ
	DirectX::XMFLOAT3 playerBulVel{};
	std::unique_ptr<Timer::timeType> playerBulStartTime;
	DirectX::XMFLOAT2 playerRota{};

	/// @brief パーティクル
	std::unique_ptr<ParticleMgr> particleMgr;

#pragma endregion 3Dオブジェクト

#pragma region シーン全体設定

	/// @brief 時間
	std::unique_ptr<Timer> timer;

	/// @brief GUI
	bool guiWinAlive = true;

	uint8_t fbxPhongNum;
	uint8_t fbxLambertNum;
	uint8_t nowFbxPSNum = FbxObj3d::ppStateNum;

	/*----- ポストエフェクト設定 -----*/

	// 透明度
	float drawAlpha = 0.f;

	// シーン遷移にかける時間
	static constexpr Timer::timeType sceneTransTime = Timer::oneSec;

#pragma endregion シーン全体設定

#pragma region シングルトンインスタンス

	Input* input = nullptr;

	DX12Base* dxBase = nullptr;

#pragma endregion シングルトンインスタンス

private:
	void createParticle(const DirectX::XMFLOAT3& pos, const UINT particleNum = 10U, const float startScale = 1.f, const float vel = 5.f);
	inline ImVec2 getWindowLBPos() { return ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y); }

	// update_何とか関数を格納する
	std::function<void()> update_proc;

	void update_start();
	void update_play();
	void update_end();

	void changeEndScene();

	void drawImGui();

#pragma region 初期化関数

	void cameraInit();

	void lightInit();

	void soundInit();

	void spriteInit();

	void obj3dInit();

	void fbxInit();

	void particleInit();

	void playerInit();

	void timerInit();

#pragma endregion 初期化関数

#pragma region 更新関数

	void updateSound();

	void updateMouse();

	void updateCamera();

	void updateLight();

	void updateSprite();

	void updatePlayer();

	void updatePlayerBullet();

	void updateBoss();

#pragma endregion 更新関数

public:

	PlayScene();
	void start() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	~PlayScene() override;
};
