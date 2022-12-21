#include "SceneManager.h"

#include "PostEffect.h"

#include "Scene/TitleScene.h"

SceneManager::SceneManager()
	: nextScene(nullptr),
	postEff2Num((UINT)PostEffect::getInstance()->addPipeLine(L"Resources/Shaders/PostEffectPS_2.hlsl"))
{
	nowScene.reset((GameScene*)new TitleScene());
	nowScene->start();
}

void SceneManager::update()
{
	// 次のシーンがあったら
	if (nextScene != nullptr)
	{
		// 今のシーンを削除し、次のシーンに入れ替える
		nowScene = std::move(nextScene);

		//次シーンの情報をクリア
		nextScene.reset();

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
}