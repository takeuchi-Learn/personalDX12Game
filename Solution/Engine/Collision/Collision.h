#pragma once

#include "CollisionShape.h"
#include <vector>

#include "3D/Obj/Object3d.h"

/// @brief 衝突判定をするクラス
class Collision
{
public:
	/// @brief ベクトルの長さを計算
	/// @param vec 対象のベクトル
	/// @return 長さ
	inline static float vecLength(const DirectX::XMVECTOR& vec)
	{
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(vec));
	}

	//private:
	inline static float vec3Dot(const DirectX::XMVECTOR& v1,
								const DirectX::XMVECTOR& v2)
	{
		float ret{};
		DirectX::XMStoreFloat(&ret, DirectX::XMVector3Dot(v1, v2));
		return ret;
	}

	/// @brief 線分同士の距離の2乗
	/// @param p1 線分1の始点
	/// @param q1 線分2の始点
	/// @param p2 線分1の終点
	/// @param q2 線分2の終点
	/// @return 距離の2乗
	static float sqDistanceSegmentSegment(const DirectX::XMVECTOR& p1, const DirectX::XMVECTOR& q1,
										  const DirectX::XMVECTOR& p2, const DirectX::XMVECTOR& q2);

public:
	/// @brief メッシュデータをポリゴン情報に変換
	/// @param obj 元となるメッシュのオブジェクト
	/// @param pPolygons ポリゴン情報を出力する変数のポインタ
	static void calcMeshTrlangles(const Object3d* obj, std::vector<Triangle>* pPolygons);

	/// @brief 球と平面の当たり判定
	/// @param sphere 球
	/// @param plane 平面
	/// @param inter 交点(平面上の最近接点)の出力用変数
	/// @return 交差しているか否か
	static bool CheckSphere2Plane(const Sphere& sphere,
								  const Plane& plane, DirectX::XMVECTOR* inter = nullptr);

	inline static bool CheckHit(const Sphere& sphere, const Plane& plane) { return CheckSphere2Plane(sphere, plane); }
	inline static bool CheckHit(const Plane& plane, const Sphere& sphere) { return CheckSphere2Plane(sphere, plane); }

	/// @brief 点と三角形の最近接点を求める
	/// @param point 点
	/// @param triangle 三角形
	/// @param closest 最近接点(出力用)
	static void ClosestPtPoint2Triangle(const DirectX::XMVECTOR& point,
										const Triangle& triangle, DirectX::XMVECTOR* closest);

	/// @brief 球と法線付き三角形の当たり判定
	/// @param sphere 球
	/// @param triangle 三角形
	/// @param inter 交点(三角形上の最近接点)
	/// @return 交差しているか否か
	static bool CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle,
									 DirectX::XMVECTOR* inter = nullptr);

	inline static bool CheckHit(const Sphere& sphere, const Triangle& triangle) { return CheckSphere2Triangle(sphere, triangle); }
	inline static bool CheckHit(const Triangle& triangle, const Sphere& sphere) { return CheckSphere2Triangle(sphere, triangle); }

	/// @brief レイと平面の当たり判定
	/// @param ray レイ
	/// @param plane 平面
	/// @param distance 距離(出力用)
	/// @param inter 交点(出力用)
	/// @return 交差しているか否か
	static bool CheckRay2Plane(const Ray& ray, const Plane& plane,
							   float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

	inline static bool CheckHit(const Ray& ray, const Plane& plane) { return CheckRay2Plane(ray, plane); }
	inline static bool CheckHit(const Plane& plane, const Ray& ray) { return CheckRay2Plane(ray, plane); }

	/// @brief レイと法線付き三角形の当たり判定
	/// @param ray レイ
	/// @param triangle 三角形
	/// @param distance 距離(出力用)
	/// @param inter 交点(出力用)
	/// @return 交差しているか否か
	static bool CheckRay2Triangle(const Ray& ray, const Triangle& triangle,
								  float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

	inline static bool CheckHit(const Ray& ray, const Triangle& triangle) { return CheckRay2Triangle(ray, triangle); }
	inline static bool CheckHit(const Triangle& triangle, const Ray& ray) { return CheckRay2Triangle(ray, triangle); }

	/// @brief レイと球の当たり判定
	/// @param ray レイ
	/// @param sphere 球
	/// @param distance 距離(出力用)
	/// @param inter 交点(出力用)
	/// @return 交差しているか否か
	static bool CheckRay2Sphere(const Ray& ray, const Sphere& sphere,
								float* distance = nullptr, DirectX::XMVECTOR* inter = nullptr);

	inline static bool CheckHit(const Ray& ray, const Sphere& sphere) { return CheckRay2Sphere(ray, sphere); }
	inline static bool CheckHit(const Sphere& sphere, const Ray& ray) { return CheckRay2Sphere(ray, sphere); }

	/// @brief と直方体(AABB)の当たり判定
	/// @param sphere 球
	/// @param aabb 直方体(AABB)
	/// @return 衝突しているか否か
	static bool CheckSphere2AABB(const Sphere& sphere, const AABB& aabb);

	inline static bool CheckHit(const Sphere& sphere, const AABB& aabb) { return CheckSphere2AABB(sphere, aabb); }
	inline static bool CheckHit(const AABB& aabb, const Sphere& sphere) { return CheckSphere2AABB(sphere, aabb); }

	/// @brief 球とカプセルの当たり判定
	/// @param sphere 球
	/// @param capsule カプセル
	/// @return 衝突しているか否か
	static bool CheckSphere2Capsule(const Sphere& sphere, const Capsule& capsule);

	inline static bool CheckHit(const Sphere& sphere, const Capsule& capsule) { return CheckSphere2Capsule(sphere, capsule); }
	inline static bool CheckHit(const Capsule& capsule, const Sphere& sphere) { return CheckSphere2Capsule(sphere, capsule); }

	/// @brief カプセル同士の当たり判定
	/// @param capsule1 カプセル
	/// @param capsule2 カプセル
	/// @return 衝突しているか否か
	static bool CheckCapsule2Capsule(const Capsule& capsule1, const Capsule& capsule2);

	inline static bool CheckHit(const Capsule& capsule, const Capsule& capsule2) { return CheckCapsule2Capsule(capsule, capsule2); }

	/// @brief 球同士の当たり判定
	/// @param sphere1 球
	/// @param sphere2 球
	/// @return 衝突しているか否か
	static bool CheckSphere2Sphere(const Sphere& sphere1, const Sphere& sphere2);

	inline static bool CheckHit(const Sphere& sphere, const Sphere& sphere2) { return CheckSphere2Sphere(sphere, sphere2); }
};
