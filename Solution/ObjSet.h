#pragma once

#include <memory>
#include <string>
#include "Object3d.h"
#include "ObjModel.h"
#include "Camera.h"
#include "Light.h"

class ObjSet
{
	std::unique_ptr<ObjModel> model;
	std::vector<Object3d> obj;

public:
	ObjSet(Camera* camera, const std::string& dirPath, const std::string& name, bool smoothing = false);

	inline ObjModel* getModelPt() { return model.get(); }

	inline void addObj(Camera* camera) { obj.emplace_back(camera, model.get(), 0U); }

	inline const DirectX::XMMATRIX& getMatWorld(UINT num = 0U) { return obj[num].getMatWorld(); }

	inline void setPos(const DirectX::XMFLOAT3& pos, UINT num = 0U) { obj[num].position = pos; }
	inline const DirectX::XMFLOAT3& getPos(UINT num = 0U) { return obj[num].position; }

	inline void setScale(const DirectX::XMFLOAT3& scale, UINT num = 0U) { obj[num].scale = scale; }
	inline const DirectX::XMFLOAT3& getScale(UINT num = 0U) { return obj[num].scale; }

	inline void setRotation(const DirectX::XMFLOAT3& rotation, UINT num = 0U) { obj[num].rotation = rotation; }
	inline const DirectX::XMFLOAT3& getRotation(UINT num = 0U) { return obj[num].rotation; }

	inline void setColor(const DirectX::XMFLOAT4& color, UINT num = 0U) { obj[num].color = color; }
	inline const DirectX::XMFLOAT4& getColor(UINT num = 0U) { return obj[num].color; }

	inline const DirectX::XMMATRIX& getMatRota(UINT num = 0U) const { return obj[num].getMatRota(); }

	void drawWithUpdate(Light* light);
};
