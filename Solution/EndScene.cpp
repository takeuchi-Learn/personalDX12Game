#include "EndScene.h"

#include "SceneManager.h"

#include "WinAPI.h"

#include "TitleScene.h"

#include "PostEffect.h"

EndScene::EndScene() {
	PostEffect::getInstance()->setAlpha(1.f);
	PostEffect::getInstance()->setMosaicNum(DirectX::XMFLOAT2(WinAPI::window_width, WinAPI::window_height));

	WinAPI::getInstance()->setWindowText("Press SPACE to change scene - now : End");

	input = Input::getInstance();

	spCom.reset(new SpriteBase());

	// デバッグテキスト用のテクスチャ読み込み
	debugTextTexNumber = spCom->loadTexture(L"Resources/debugfont.png");

	debugText.reset(new DebugText(debugTextTexNumber, spCom.get()));
}

void EndScene::update() {
	if (input->triggerKey(DIK_SPACE)) {
		SceneManager::getInstange()->changeScene(new TitleScene());
	}

	debugText->Print(spCom.get(), "END", 0, 0, 10.f);
}

void EndScene::drawFrontSprite() {
	spCom->drawStart(DX12Base::getInstance()->getCmdList());
	debugText->DrawAll(DX12Base::getInstance(), spCom.get());
}
