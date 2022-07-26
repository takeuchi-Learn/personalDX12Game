#pragma once

#include <memory>
#include <string>
#include "Object3d.h"
#include "ObjModel.h"
#include "Camera.h"
#include "Light.h"

class ObjSet {
	std::unique_ptr<ObjModel> model;
	std::unique_ptr<Object3d> obj;

public:
	ObjSet(Camera* camera, const std::string &dirPath, const std::string &name, bool smoothing = false);

	inline void setPos(const DirectX::XMFLOAT3 &pos) { obj->position = pos; }
	inline const DirectX::XMFLOAT3 &getPos() { return obj->position; }

	inline void setScale(const DirectX::XMFLOAT3 &scale) { obj->scale = scale; }
	inline const DirectX::XMFLOAT3 &getScale() { return obj->scale; }

	inline void setRotation(const DirectX::XMFLOAT3 &rotation) { obj->rotation = rotation; }
	inline const DirectX::XMFLOAT3 &getRotation() { return obj->rotation; }

	inline void setColor(const DirectX::XMFLOAT4 &color) { obj->color = color; }
	inline const DirectX::XMFLOAT4 &getColor() { return obj->color; }

	void drawWithUpdate(Light* light);
};

