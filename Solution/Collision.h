#pragma once

#include "CollisionShape.h"

class Collision {
private:
	inline static float vecLength(DirectX::XMVECTOR vec) {
		float len{};
		DirectX::XMStoreFloat(&len, vec);
		return len;
	}

	inline static float vec3Dot(const DirectX::XMVECTOR &v1,
								const DirectX::XMVECTOR &v2) {
		float ret{};
		DirectX::XMStoreFloat(&ret, DirectX::XMVector3Dot(v1, v2));
		return ret;
	}

	inline static float clamp(float x, float low, float high) {
		x = (x < low) ? low : x;
		x = (x < high) ? high : x;
		return x;
	}

	// 線分同士の距離の2乗
	static float sqDistanceSegmentSegment(const DirectX::XMVECTOR &p1, const DirectX::XMVECTOR &q1,
										  const DirectX::XMVECTOR &p2, const DirectX::XMVECTOR &q2);

public:
	/// <summary>
	/// 球と平面の当たり判定
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="plane">平面</param>
	/// <param name="inter">交点(平面上の最近接点)</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckSphere2Plane(const Sphere &sphere,
								  const Plane &plane, DirectX::XMVECTOR *inter = nullptr);

	/// <summary>
	/// 点と三角形の最近接点を求める
	/// </summary>
	/// <param name="point">点</param>
	/// <param name="triangle">三角形</param>
	/// <param name="closest">最近接点(出力用)</param>
	static void ClosestPtPoint2Triangle(const DirectX::XMVECTOR &point,
										const Triangle &triangle, DirectX::XMVECTOR *closest);

	/// <summary>
	/// 球と法線付き三角形の当たり判定
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="triangle">三角形</param>
	/// <param name="inter">交点(三角形上の最近接点)</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckSphere2Triangle(const Sphere &sphere, const Triangle &triangle,
									 DirectX::XMVECTOR *inter = nullptr);

	/// <summary>
	/// レイと平面の当たり判定
	/// </summary>
	/// <param name="ray">レイ</param>
	/// <param name="plane">平面</param>
	/// <param name="distance">距離(出力用)</param>
	/// <param name="inter">交点(出力用)</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckRay2Plane(const Ray &ray, const Plane &plane,
							   float *distance = nullptr, DirectX::XMVECTOR *inter = nullptr);

	/// <summary>
	/// レイと法線付き三角形の当たり判定
	/// </summary>
	/// <param name="ray">レイ</param>
	/// <param name="triangle">三角形</param>
	/// <param name="distance">距離(出力用)</param>
	/// <param name="inter">交点(出力用)</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckRay2Triangle(const Ray &ray, const Triangle &triangle,
								  float *distance = nullptr, DirectX::XMVECTOR *inter = nullptr);

	/// <summary>
	/// レイと球の当たり判定
	/// </summary>
	/// <param name="ray">レイ</param>
	/// <param name="sphere">球</param>
	/// <param name="distance">距離(出力用)</param>
	/// <param name="inter">交点(出力用)</param>
	/// <returns>交差しているか否か</returns>
	static bool CheckRay2Sphere(const Ray &ray, const Sphere &sphere,
								float *distance = nullptr, DirectX::XMVECTOR *inter = nullptr);

	/// <summary>
	/// 球と直方体(AABB)の当たり判定
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="aabb">直方体(AABB)</param>
	/// <returns>衝突しているか否か</returns>
	static bool CheckSphere2AABB(const Sphere &sphere, const AABB &aabb);

	/// <summary>
	/// 球とカプセルの当たり判定
	/// </summary>
	/// <param name="sphere">球</param>
	/// <param name="capsule">カプセル</param>
	/// <returns>衝突しているか否か</returns>
	static bool CheckSphere2Capsule(const Sphere &sphere, const Capsule &capsule);

	/// <summary>
	/// カプセル同士の当たり判定
	/// </summary>
	/// <param name="capsule1">カプセル</param>
	/// <param name="capsule2">カプセル</param>
	/// <returns>衝突しているか否か</returns>
	static bool CheckCapsule2Capsule(const Capsule &capsule1, const Capsule &capsule2);

	/// <summary>
	/// 球同士の当たり判定
	/// </summary>
	/// <param name="sphere1">球</param>
	/// <param name="sphere2">球</param>
	/// <returns>衝突しているか否か</returns>
	static bool CheckSphere2Sphere(const Sphere &sphere1, const Sphere &sphere2);
};
