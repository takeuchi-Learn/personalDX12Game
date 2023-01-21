﻿#pragma once
#include <DirectXMath.h>
#include <memory>
#include "System/DX12Base.h"
#include "3D/Light.h"
#include "3D/Obj/Object3d.h"

/// @brief ゲームオブジェクト基底クラス
class GameObj
{
protected:
	std::unique_ptr<Object3d> obj;

	bool alive = true;
	bool drawFlag = true;

	virtual void additionalUpdate() {};
	virtual void additionalDraw(Light* light) {}

public:
	inline static DirectX::XMFLOAT2 calcRotationSyncVelRad(const DirectX::XMFLOAT3& vel)
	{
		return DirectX::XMFLOAT2(std::atan2(-vel.y,
											std::sqrt(vel.x * vel.x +
													  vel.z * vel.z)),
								 std::atan2(vel.x, vel.z));
	}
	inline static DirectX::XMFLOAT2 calcRotationSyncVelDeg(const DirectX::XMFLOAT3& vel)
	{
		const DirectX::XMFLOAT2 rad = calcRotationSyncVelRad(vel);
		return DirectX::XMFLOAT2(DirectX::XMConvertToDegrees(rad.x), DirectX::XMConvertToDegrees(rad.y));
	}

	inline static DirectX::XMFLOAT3 calcVel(const DirectX::XMFLOAT3& targetPos,
											const DirectX::XMFLOAT3& nowPos,
											float velScale)
	{
		DirectX::XMFLOAT3 velF3{
			targetPos.x - nowPos.x,
			targetPos.y - nowPos.y,
			targetPos.z - nowPos.z
		};

		const DirectX::XMVECTOR velVec =
			DirectX::XMVectorScale(
				DirectX::XMVector3Normalize(
					DirectX::XMLoadFloat3(&velF3)
				),
				velScale
			);

		DirectX::XMStoreFloat3(&velF3, velVec);
		return velF3;
	}

#pragma region アクセッサ

	inline BaseObj* getParent() { return obj->parent; }
	inline void setParent(BaseObj* parent) { obj->parent = parent; }

	inline bool getAlive() const { return alive; }
	inline void setAlive(bool alive) { this->alive = alive; }
	/// @brief aliveをfalseにする
	inline void kill() { alive = false; }

	inline bool getDrawFlag() const { return drawFlag; }
	inline void setDrawFlag(bool drawFlag) { this->drawFlag = drawFlag; }

	inline Object3d* getObj() const { return obj.get(); }

	inline void setPos(const DirectX::XMFLOAT3& pos) { obj->position = pos; }
	inline const DirectX::XMFLOAT3& getPos() const { return obj->position; }

	inline void setCol(const DirectX::XMFLOAT4& col) { obj->color = col; }
	inline const DirectX::XMFLOAT4& getCol() const { return obj->color; }

	inline void setScaleF3(const DirectX::XMFLOAT3& scale) { obj->scale = scale; }
	inline void setScale(float scale) { obj->scale = DirectX::XMFLOAT3(scale, scale, scale); }
	inline const DirectX::XMFLOAT3& getScaleF3() const { return obj->scale; }

	/// @return Zのスケールを返す
	inline float getScale() const { return obj->scale.z; }

	inline const DirectX::XMFLOAT3& getRotation() const { return obj->rotation; }
	inline void setRotation(const DirectX::XMFLOAT3& rota) { obj->rotation = rota; }

	inline const DirectX::XMMATRIX& getMatWorld() const { return obj->getMatWorld(); }
	inline const DirectX::XMMATRIX& getMatRota() const { return obj->getMatRota(); }
	inline const DirectX::XMMATRIX& getMatScale() const { return obj->getMatScale(); }
	inline const DirectX::XMMATRIX& getMatTrans() const { return obj->getMatTrans(); }

#pragma endregion アクセッサ

	/// @return 大きさベクトルの長さを返す
	inline float calcScale() const
	{
		return std::sqrt(obj->scale.x * obj->scale.x +
						 obj->scale.y * obj->scale.y +
						 obj->scale.z * obj->scale.z);
	}

	inline DirectX::XMFLOAT3 calcWorldPos() const { return obj->calcWorldPos(); }

	GameObj(Camera* camera,
			ObjModel* model,
			const DirectX::XMFLOAT3& pos = { 0,0,0 });
	~GameObj();

	// drawWithUpdate関数の頭で呼ばれる
	void update();

	// drawWithUpdate関数で呼ばれる
	void draw(Light* light);

	void drawWithUpdate(Light* light);
};
