#include "EndScene.h"

#include "TitleScene.h"
#include "../Engine/System/PostEffect.h"
#include "../Engine/System/SceneManager.h"

EndScene::EndScene()
{
	PostEffect::getInstance()->setAlpha(1.f);
	PostEffect::getInstance()->setMosaicNum(DirectX::XMFLOAT2(WinAPI::window_width, WinAPI::window_height));

	input = Input::getInstance();

	spCom.reset(new SpriteBase());

	// デバッグテキスト用のテクスチャ読み込み
	debugTextTexNumber = spCom->loadTexture(L"Resources/debugfont.png");

	debugText.reset(new DebugText(debugTextTexNumber, spCom.get()));

	end.reset(new Sprite(spCom->loadTexture(L"Resources/end.png"),
						 spCom.get(),
						 DirectX::XMFLOAT2(0.f, 0.f)));
}

void EndScene::update()
{
	if (input->triggerKey(DIK_SPACE))
	{
		SceneManager::getInstange()->changeScene(new TitleScene());
	}
}

void EndScene::drawFrontSprite()
{
	spCom->drawStart(DX12Base::getInstance()->getCmdList());
	end->drawWithUpdate(DX12Base::ins(), spCom.get());
}