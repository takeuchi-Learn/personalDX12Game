#include "GameObject.h"
#include "DX12Base.h"

GameObject::GameObject(std::unique_ptr<CollisionShape> &&col,
					   std::unique_ptr<Object3d> &&obj3d,
					   std::unique_ptr<ObjModel> &&model)
	: collisionShape(std::move(col)),
	obj3d(std::move(obj3d)),
	model(std::move(model)) {
}

void GameObject::drawWithUpdate(Light *light) {
	if (alive) obj3d->drawWithUpdate(DX12Base::getInstance(), light);
}
