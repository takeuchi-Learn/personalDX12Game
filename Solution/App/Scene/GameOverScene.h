/*****************************************************************//**
 * \file   GameOverScene.h
 * \brief  死んだときに来るシーン
 *********************************************************************/

#pragma once

#include <System/GameScene.h>
#include <2D/Sprite.h>

 /// @brief 死んだときに来るシーンのクラス
class GameOverScene :
	public GameScene
{
	std::unique_ptr<SpriteBase> spBase;
	std::unique_ptr<Sprite> gameOverGr;

public:
	GameOverScene();
	void update() override;
	void drawFrontSprite() override;
};
