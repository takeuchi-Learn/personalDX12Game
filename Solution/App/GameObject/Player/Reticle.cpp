#include "Reticle.h"

Reticle::Reticle(UINT texNum,
				 const SpriteBase* spBase) :
	sprite(std::make_unique<Sprite>(texNum, spBase))
{
}

void Reticle::update(const SpriteBase* spBase)
{
	if (sprite->isInvisible) { return; }

	if (target.expired())
	{
		sprite->isInvisible = true;
		return;
	}
	auto& i = target.lock();

	if (!i->getAlive())
	{
		sprite->isInvisible = true;
		return;
	}

	sprite->position = i->getObj()->calcScreenPosF3();
	sprite->update(spBase);
}

void Reticle::draw(const SpriteBase* spBase)
{
	sprite->draw(DX12Base::ins()->getCmdList(), spBase, DX12Base::ins()->getDev());
}