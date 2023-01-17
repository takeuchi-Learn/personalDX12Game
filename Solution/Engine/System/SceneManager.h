/*****************************************************************//**
 * \file   SceneManager.h
 * \brief  シーンの管理をするクラス
 *********************************************************************/

#pragma once
#include "GameScene.h"
#include <memory>

 /// @brief シーンの管理をする
class SceneManager
	: public GameScene
{
private:
	SceneManager(const SceneManager& sm) = delete;
	SceneManager& operator=(const SceneManager& sm) = delete;
	SceneManager();

	std::unique_ptr<GameScene> nowScene;
	std::unique_ptr<GameScene> nextScene;

	using UINT = unsigned int;

	UINT postEff2Num = 0U;

public:
	inline UINT getPostEff2Num() { return postEff2Num; }

	inline static SceneManager* getInstange()
	{
		static std::unique_ptr<SceneManager> sm(new SceneManager());
		return sm.get();
	}

	~SceneManager() override;

	void update() override;
	void drawObj3d() override;
	void drawFrontSprite() override;

	template <class SCENE>
	inline void changeScene() { nextScene = std::make_unique<SCENE>(); }

	inline void changeSceneFromInstance(std::unique_ptr<GameScene>& nextScene)
	{
		this->nextScene = std::move(nextScene);
	}
};
