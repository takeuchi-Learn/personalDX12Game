#include "GameOverScene.h"

#include <Input/Input.h>
#include <System/SceneManager.h>
#include "TitleScene.h"
#include <DirectXMath.h>

using namespace DirectX;

GameOverScene::GameOverScene() :
	spBase(std::make_unique<SpriteBase>())
{
	gameOverGr = std::make_unique<Sprite>(spBase->loadTexture(L"Resources/GameOverScene/gameOverGr.png"),
										  spBase.get(),
										  XMFLOAT2(0.f, 0.f));
}

void GameOverScene::update()
{
	if (Input::ins()->triggerKey(DIK_SPACE) ||
		Input::ins()->triggerPadButton(Input::PAD::A) ||
		Input::ins()->triggerPadButton(Input::PAD::B))
	{
		SceneManager::getInstange()->changeScene<TitleScene>();
	}
}

void GameOverScene::drawFrontSprite()
{
	spBase->drawStart(DX12Base::ins()->getCmdList());
	gameOverGr->drawWithUpdate(DX12Base::ins(), spBase.get());
}