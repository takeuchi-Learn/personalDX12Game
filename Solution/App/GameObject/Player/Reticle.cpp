#include "Reticle.h"

Reticle::Reticle(UINT texNum,
				 const SpriteBase* spBase) :
	sprite(std::make_unique<Sprite>(texNum, spBase))
{}

void Reticle::update(const SpriteBase* spBase)
{
	if (sprite->isInvisible) { return; }

	if (target.expired())
	{
		sprite->isInvisible = true;
		return;
	}
	std::shared_ptr<GameObj>& i = target.lock();

	if (!i->getAlive() || i->getObj().expired())
	{
		sprite->isInvisible = true;
		return;
	}

	sprite->position = i->getObj().lock()->calcScreenPosF3();
	sprite->update(spBase);
}

void Reticle::draw(const SpriteBase* spBase)
{
	sprite->draw(DX12Base::ins()->getCmdList(), spBase, DX12Base::ins()->getDev());
}