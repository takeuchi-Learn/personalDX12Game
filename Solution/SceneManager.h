#pragma once
#include "GameScene.h"

class SceneManager
	: public GameScene {
private:
	SceneManager(const SceneManager& sm) = delete;
	SceneManager& operator=(const SceneManager& sm) = delete;
	SceneManager();

	GameScene* nowScene = nullptr;
	GameScene* nextScene = nullptr;

	using UINT = unsigned int;

	UINT postEff2Num = 0U;

public:
	inline UINT getPostEff2Num() { return postEff2Num; }

	static SceneManager* getInstange();

	~SceneManager() override;

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	void changeScene(GameScene* nextScene);
};

