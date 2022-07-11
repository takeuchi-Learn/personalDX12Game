#include "SceneManager.h"

#include "TitleScene.h"

#include "Input.h"

SceneManager::SceneManager()
	: nextScene(nullptr) {

	nowScene = (GameScene *)new TitleScene();
	nowScene->init();
}

SceneManager *SceneManager::getInstange() {
	static SceneManager sm;
	return &sm;
}


void SceneManager::update() {

	// 次のシーンがあったら
	if (nextScene != nullptr) {

		// 今のシーンを削除し、次のシーンに入れ替える
		delete nowScene;
		nowScene = nextScene;

		// 次のシーンの初期化処理
		nextScene->init();

		//次シーンの情報をクリア
		nextScene = nullptr;
	}

	nowScene->update();
}

void SceneManager::drawObj3d() {
	nowScene->drawObj3d();
}

void SceneManager::drawFrontSprite() {
	nowScene->drawFrontSprite();
}

SceneManager::~SceneManager() {
	if (nowScene != nullptr) {
		delete nowScene;
		nowScene = nullptr;
	}
}

void SceneManager::changeScene(GameScene* nextScene) {
	this->nextScene = nextScene;
}
