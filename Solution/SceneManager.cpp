#include "SceneManager.h"

#include "TitleScene.h"

#include "Input.h"
#include "PostEffect.h"
#include "SoundBase.h"

SceneManager::SceneManager()
	: nextScene(nullptr)
{
	postEff2Num = (UINT)PostEffect::getInstance()->addPipeLine(L"Resources/Shaders/PostEffectPS_2.hlsl");

	nowScene = (GameScene*)new TitleScene();
	nowScene->start();
}

void SceneManager::update()
{
	// 次のシーンがあったら
	if (nextScene != nullptr)
	{
		// 今のシーンを削除し、次のシーンに入れ替える
		delete nowScene;
		nowScene = nextScene;

		//次シーンの情報をクリア
		nextScene = nullptr;

		// 次のシーンの初期化処理
		nowScene->start();
	}

	nowScene->update();
}

void SceneManager::drawObj3d()
{
	nowScene->drawObj3d();
}

void SceneManager::drawFrontSprite()
{
	nowScene->drawFrontSprite();
}

SceneManager::~SceneManager()
{
	if (nowScene != nullptr)
	{
		delete nowScene;
		nowScene = nullptr;
	}
}

void SceneManager::changeScene(GameScene* nextScene)
{
	this->nextScene = nextScene;
}