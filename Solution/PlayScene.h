#pragma once
#include "GameScene.h"

#include <memory>
#include "Time.h"
#include "Sound.h"
#include "Sprite.h"
#include "Object3d.h"
#include "DebugText.h"

#include <DirectXMath.h>

#include "Camera.h"

#include "ParticleMgr.h"

#include "DX12Base.h"

#include "Input.h"

#include <vector>

#include "Light.h"

#include "FbxObj3d.h"

#include <functional>

#include "Player.h"

#include "ObjSet.h"
#include <utility>

class PlayScene :
	public GameScene {

#pragma region カメラ

	std::unique_ptr<Camera> camera;
	DirectX::XMFLOAT2 cameraMoveVel{};

#pragma endregion カメラ

#pragma region 音

	std::unique_ptr<SoundBase> soundBase;

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
	Object3d::PipelineSet object3dPipelineSet;
	Object3d::PipelineSet backPipelineSet;

	/// <summary>
	///  天球
	/// </summary>
	std::unique_ptr<ObjSet> back;

	/// <summary>
	///  地面(背景)
	/// </summary>
	std::unique_ptr<ObjSet> ground;

	/// <summary>
	/// ボス
	/// </summary>
	std::unique_ptr<ObjSet> boss;
	std::unique_ptr<Time> bossTimer;
	bool bossAlive = true;

	/// <summary>
	/// 自機
	/// </summary>
	std::unique_ptr<Player> player;
	std::pair<std::unique_ptr<ObjSet>, bool> playerBul;	// second : 生存フラグ
	DirectX::XMFLOAT3 playerBulVel{};
	std::unique_ptr<Time> playerBulTimer;
	DirectX::XMFLOAT2 playerRota{};

	std::unique_ptr<FbxModel> playerFbxModel;
	std::unique_ptr<FbxObj3d> playerFbxObj3d;

#pragma endregion 3Dオブジェクト

	/// <summary>
	/// パーティクル
	/// </summary>
	std::unique_ptr<ParticleMgr> particleMgr;
	std::unique_ptr<Time> particleTimer;

	/// <summary>
	/// 時間
	/// </summary>
	std::unique_ptr<Time> timer;

	/// <summary>
	/// GUI
	/// </summary>
	bool guiWinAlive = true;

#pragma region ポストエフェクトの設定
	// 透明度
	float drawAlpha = 0.f;

	// シーン遷移にかける時間
	Time::timeType sceneTransTime = Time::oneSec;

	// 現在のポストエフェクトの管理番号格納変数
	UINT postEff2Num = 0u;

#pragma endregion ポストエフェクトの設定

#pragma region シングルトンインスタンス

	Input *input = nullptr;

	DX12Base *dxBase = nullptr;

#pragma endregion シングルトンインスタンス


private:
	void createParticle(const DirectX::XMFLOAT3 &pos, const UINT particleNum = 10U, const float startScale = 1.f, const float vel = 5.f);
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
	void init() override;
	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	~PlayScene() override;
};

