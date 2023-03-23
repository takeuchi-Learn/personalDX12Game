#pragma once
#include <2D/Sprite.h>
#include <GameObject/GameObj.h>

class Reticle
{
public:
	std::unique_ptr<Sprite> sprite;
	std::weak_ptr<GameObj> target;

	Reticle(UINT texNum, const SpriteBase* spBase);

	void update(const SpriteBase* spBase);
	void draw(const SpriteBase* spBase);

	inline void drawWithUpdate(const SpriteBase* spBase)
	{
		update(spBase);
		draw(spBase);
	}
};

