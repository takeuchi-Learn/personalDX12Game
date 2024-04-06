#include "GameObj.h"

using namespace DirectX;

bool GameObj::damage(uint16_t damegeNum, bool killFlag)
{
	if (damegeNum >= hp)
	{
		hp = 0u;
		if (killFlag) { kill(); }
		return true;
	}

	hp -= damegeNum;
	return false;
}

void GameObj::moveForward(float moveVel, bool moveYFlag)
{
	// Z方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, 0, moveVel, 1), obj->getMatRota());

	// Y方向に移動しないならY成分を消す
	if (!moveYFlag)
	{
		// absがあるのは、大きさのみ指定したいから。
		// absがないと、moveVelがマイナスの場合に
		// マイナス * マイナスでプラスになってしまう
		velVec = XMVectorScale(XMVector3Normalize(XMVectorSetY(velVec, 0.f)),
							   std::abs(moveVel));
	}

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void GameObj::moveRight(float moveVel, bool moveYFlag)
{
	// X方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(moveVel, 0, 0, 1), obj->getMatRota());

	// Y方向に移動しないならY成分を消す
	if (!moveYFlag)
	{
		// absがあるのは、大きさのみ指定したいから。
		// absがないと、moveVelがマイナスの場合に
		// マイナス * マイナスでプラスになってしまう
		velVec = XMVectorScale(XMVector3Normalize(XMVectorSetY(velVec, 0.f)),
							   std::abs(moveVel));
	}

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void GameObj::moveParentRight(float moveVel, bool moveYFlag)
{
	if (!obj->parent)
	{
		moveRight(moveVel, moveYFlag);
		return;
	}

	// X方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(moveVel, 0, 0, 1), obj->parent->getMatRota());

	// Y方向に移動しないならY成分を消す
	if (!moveYFlag)
	{
		// absがあるのは、大きさのみ指定したいから。
		// absがないと、moveVelがマイナスの場合に
		// マイナス * マイナスでプラスになってしまう
		velVec = XMVectorScale(XMVector3Normalize(XMVectorSetY(velVec, 0.f)),
							   std::abs(moveVel));
	}

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void GameObj::moveUp(float moveVel)
{
	// Y方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, moveVel, 0, 1), obj->getMatRota());

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

void GameObj::moveParentUp(float moveVel)
{
	if (!obj->parent)
	{
		moveUp(moveVel);
		return;
	}

	// Y方向のベクトルを、自機の向いている向きに回転
	XMVECTOR velVec = XMVector3Transform(XMVectorSet(0, moveVel, 0, 1), obj->parent->getMatRota());

	obj->position.x += XMVectorGetX(velVec);
	obj->position.y += XMVectorGetY(velVec);
	obj->position.z += XMVectorGetZ(velVec);
}

GameObj::GameObj(Camera* camera,
				 ObjModel* model,
				 const DirectX::XMFLOAT3& pos)
	: objObject(std::make_unique<Object3d>(camera,
										   model)),
	ppStateNum(Object3d::ppStateNum)
{
	obj = objObject.get();
	setPos(pos);
}

GameObj::GameObj(Camera* camera)
	: objObject(std::make_unique<Object3d>(camera, nullptr)),
	ppStateNum(Object3d::ppStateNum)
{
	obj = objObject.get();
}

GameObj::~GameObj()
{
	objObject.reset(nullptr);
}

void GameObj::update()
{
	additionalUpdate();
	obj->update();
}

void GameObj::draw(Light* light)
{
	if (drawFlag)
	{
		obj->draw(light, ppStateNum);
	}
	additionalDraw(light);
}

void GameObj::drawWithUpdate(Light* light)
{
	update();

	draw(light);

	additionalDraw(light);
}