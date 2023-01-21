﻿#include "GameObj.h"

using namespace DirectX;

GameObj::GameObj(Camera* camera,
				 ObjModel* model,
				 const DirectX::XMFLOAT3& pos)
	: objObject(std::make_unique<Object3d>(camera,
										   model))
{
	obj = objObject.get();
	setPos(pos);
}

GameObj::GameObj(Camera* camera,
				 FbxModel* model,
				 const DirectX::XMFLOAT3& pos)
	: fbxObject(std::make_unique<FbxObj3d>(camera,
										  model))
{
	obj = fbxObject.get();
	setPos(pos);
}

GameObj::GameObj(Camera* camera)
	: objObject(std::make_unique<Object3d>(camera, nullptr))
{
	obj = objObject.get();
}

GameObj::~GameObj()
{
	objObject.reset(nullptr);
	fbxObject.reset(nullptr);
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
		obj->draw(light, BaseObj::ppStateNum);
	}
	additionalDraw(light);
}

void GameObj::drawWithUpdate(Light* light)
{
	update();

	draw(light);

	additionalDraw(light);
}