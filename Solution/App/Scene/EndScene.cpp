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
}

void EndScene::update()
{
	if (input->triggerKey(DIK_SPACE))
	{
		SceneManager::getInstange()->changeScene(new TitleScene());
	}

	debugText->Print(spCom.get(), "END", 0, 0, 10.f);
	debugText->Print(spCom.get(), "Press SPACE...", 0.f, WinAPI::window_height / 2.f, 1.f);
}

void EndScene::drawFrontSprite()
{
	spCom->drawStart(DX12Base::getInstance()->getCmdList());
	debugText->DrawAll(DX12Base::getInstance(), spCom.get());
}