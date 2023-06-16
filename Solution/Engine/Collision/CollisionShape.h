#pragma once

// 当たり判定プリミティブ

#include <DirectXMath.h>

namespace CollisionShape
{
	struct BaseShape {};

	// 球
	struct Sphere :
		public BaseShape
	{
		// 中心座標
		DirectX::XMVECTOR center = { 0, 0, 0, 1 };
		// 半径
		float radius = 1.f;

		Sphere(const DirectX::XMVECTOR& center = { 0,0,0,1 }, float r = 1.f)
			: center(center), radius(r)
		{}
	};

	// 平面
	struct Plane :
		public BaseShape
	{
		// 法線ベクトル
		DirectX::XMVECTOR normal = { 0, 1, 0, 0 };
		// 原点(0, 0, 0)からの距離
		float distance = 0.f;

		Plane(const DirectX::XMVECTOR& normal = { 0, 1, 0, 0 },
			  float distance = 0.f)
			: normal(normal), distance(distance)
		{}
	};

	// 法線付き三角形(時計回りが表)
	class Triangle :
		public BaseShape
	{
	public:
		//頂点情報3つ
		DirectX::XMVECTOR p0;
		DirectX::XMVECTOR p1;
		DirectX::XMVECTOR p2;
		//法線ベクトル
		DirectX::XMVECTOR normal;

		Triangle(const DirectX::XMVECTOR& p0 = { -1, -1, 0, 1 },
				 const DirectX::XMVECTOR& p1 = { +1, -1, 0, 1 },
				 const DirectX::XMVECTOR& p2 = { 0, 1, 0, 1 },
				 const DirectX::XMVECTOR& normal = { 0, 0, 1, 1 })
			:p0(p0), p1(p1), p2(p2), normal(normal)
		{}

		// 法線の計算
		void ComputeNormal();
	};

	// レイ(半直線)
	struct Ray :
		public BaseShape
	{
		// 始点
		DirectX::XMVECTOR start = { 0, 0, 0, 1 };
		// 方向
		DirectX::XMVECTOR dir = { 1, 0, 0, 0 };

		Ray(const DirectX::XMVECTOR& start = { 0, 0, 0, 1 },
			const DirectX::XMVECTOR& dir = { 1, 0, 0, 0 })
			: start(start), dir(dir)
		{}
	};

	// 直方体(AABB)
	struct AABB :
		public BaseShape
	{
		DirectX::XMVECTOR minPos{};
		DirectX::XMVECTOR maxPos{ 1, 1, 1, 1 };

		AABB(const DirectX::XMVECTOR& minPos = { 0, 0, 0, 1 },
			 const DirectX::XMVECTOR& maxPos = { 1, 1, 1, 1 })
			:minPos(minPos), maxPos(maxPos)
		{}
	};

	// カプセル
	struct Capsule :
		public BaseShape
	{
		DirectX::XMVECTOR startPos{};
		DirectX::XMVECTOR endPos{ 0, 1, 0, 1 };
		float radius = 1.f;

		Capsule(const DirectX::XMVECTOR& sPos = { 0, 0, 0, 1 },
				const DirectX::XMVECTOR& ePos = { 0, 1, 0, 1 },
				float r = 1.f)
			: startPos(sPos), endPos(ePos), radius(r)
		{}
	};
}

#ifndef NO_USING_NAMESPACE_COLLISION_SHAPE

using namespace CollisionShape;

#endif // NO_USING_NAMESPACE_COLLISION_SHAPE
