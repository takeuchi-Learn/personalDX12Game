#include "Collision.h"
#include <algorithm>

using namespace DirectX;
using namespace CollisionShape;

float Collision::sqDistanceSegmentSegment(const XMVECTOR& p1, const XMVECTOR& q1,
										  const XMVECTOR& p2, const XMVECTOR& q2)
{
	const XMVECTOR d1 = q1 - p1;	// p1->q1
	const XMVECTOR d2 = q2 - p2;	// p2->q2

	const XMVECTOR r = p1 - p2;

	const float a = vec3Dot(d1, d1);	// d1 * d1
	const float b = vec3Dot(d1, d2);	// d1 * d2
	const float e = vec3Dot(d2, d2);	// d2 * d2

	const float c = vec3Dot(d1, r);	// d1 * r
	const float f = vec3Dot(d2, r);	// d2 * r

	float s = 0.f;
	float t = 0.f;

	const float denominator = a * e - (b * b);

	// sを求める
	if (denominator != 0.f)
	{	// 平行でないなら
		s = (b * f - c * e) / denominator;
		s = std::clamp(s, 0.f, 1.f);
	} else
	{
		s = 0.f;
	}

	// tを求める
	// t = ((p1 + d1 * s) - p2) * d2 / (d2 * d2)
	t = (f + b * s) / e;

	// tが[0, 1]の範囲外ならsを再計算
	// s = ((p2 + d2 * t) - p1) * d1 / (d1 * d1) = (t * b - c) / a
	if (t < 0.f)
	{
		t = 0.f;
		s = std::clamp(-c / a, 0.f, 1.f);
	} else if (t > 1.f)
	{
		t = 1.f;
		s = std::clamp((b - c) / a, 0.f, 1.f);
	}

	// sとtが決定したので、各線分内の座標c1,c2を求める
	const XMVECTOR c1 = p1 + s * d1;
	const XMVECTOR c2 = p2 + t * d2;

	// 2点(c1, c2)間の距離の2乗を返す
	const XMVECTOR vlen = c1 - c2;
	return vec3Dot(vlen, vlen);	// vlen・vlen <- 各要素をそれぞれ掛けて全て足す
}

void Collision::calcMeshTrlangles(const Object3d* obj, std::vector<Triangle>* pPolygons)
{
	// todo アフィン変換情報が反映されていないので反映させる

	// 出力変数が無ければ何もせずに終了
	if (pPolygons == nullptr) { return; }

	std::vector<Triangle>& triangles = *pPolygons;

	const std::vector<Mesh*>& mesh = obj->model->getMesh();

	for (auto& i : mesh)
	{
		const std::vector<Mesh::VertexPosNormalUv>& vertices = i->getVertices();
		const std::vector<unsigned short>& indices = i->getIndices();

		const size_t triNum = indices.size() / 3u;

		triangles.resize(triangles.size() + triNum);

		for (size_t i = 0u; i < triNum; ++i)
		{
			Triangle& tri = triangles[i];
			unsigned short ind0 = indices[(size_t)i * 3];
			unsigned short ind1 = indices[(size_t)i * 3 + 1];
			unsigned short ind2 = indices[(size_t)i * 3 + 2];

			tri.p0 = {
				vertices[ind0].pos.x,
				vertices[ind0].pos.y,
				vertices[ind0].pos.z,
				1
			};

			tri.p1 = {
				vertices[ind1].pos.x,
				vertices[ind1].pos.y,
				vertices[ind1].pos.z,
				1
			};

			tri.p2 = {
				vertices[ind2].pos.x,
				vertices[ind2].pos.y,
				vertices[ind2].pos.z,
				1
			};

			tri.ComputeNormal();
		}
	}
}

bool Collision::CheckSphere2Plane(const Sphere& sphere, const Plane& plane, DirectX::XMVECTOR* inter)
{
	//座標系の原点から球の中心への座標
	const XMVECTOR distV = XMVector3Dot(sphere.center, plane.normal);
	//平面の原点距離を計算すると平面と球の中心との距離が出る
	const float dist = distV.m128_f32[0] - plane.distance;
	//距離の絶対値が半径より大きければ当たっていない
	if (fabsf(dist) > sphere.radius) return false;

	//疑似交点を計算
	if (inter)
	{
		//平面上の最近接点を疑似交点とする
		*inter = -dist * plane.normal + sphere.center;
	}

	return true;
}

void Collision::ClosestPtPoint2Triangle(const DirectX::XMVECTOR& point,
										const Triangle& triangle, DirectX::XMVECTOR* closest)
{
	// pointがp0の外側の頂点領域の中にあるかどうかチェック
	const XMVECTOR p0_p1 = triangle.p1 - triangle.p0;
	const XMVECTOR p0_p2 = triangle.p2 - triangle.p0;
	const XMVECTOR p0_pt = point - triangle.p0;

	const XMVECTOR d1 = XMVector3Dot(p0_p1, p0_pt);
	const XMVECTOR d2 = XMVector3Dot(p0_p2, p0_pt);

	if (d1.m128_f32[0] <= 0.0f && d2.m128_f32[0] <= 0.0f)
	{
		// p0が最近傍
		*closest = triangle.p0;
		return;
	}

	// pointがp1の外側の頂点領域の中にあるかどうかチェック
	const XMVECTOR p1_pt = point - triangle.p1;

	const XMVECTOR d3 = XMVector3Dot(p0_p1, p1_pt);
	const XMVECTOR d4 = XMVector3Dot(p0_p2, p1_pt);

	if (d3.m128_f32[0] >= 0.0f && d4.m128_f32[0] <= d3.m128_f32[0])
	{
		// p1が最近傍
		*closest = triangle.p1;
		return;
	}

	// pointがp0_p1の辺領域の中にあるかどうかチェックし、あればpointのp0_p1上に対する射影を返す
	const float vc = d1.m128_f32[0] * d4.m128_f32[0] - d3.m128_f32[0] * d2.m128_f32[0];
	if (vc <= 0.0f && d1.m128_f32[0] >= 0.0f && d3.m128_f32[0] <= 0.0f)
	{
		float v = d1.m128_f32[0] / (d1.m128_f32[0] - d3.m128_f32[0]);
		*closest = triangle.p0 + v * p0_p1;
		return;
	}

	// pointがp2の外側の頂点領域の中にあるかどうかチェック
	const XMVECTOR p2_pt = point - triangle.p2;

	const XMVECTOR d5 = XMVector3Dot(p0_p1, p2_pt);
	const XMVECTOR d6 = XMVector3Dot(p0_p2, p2_pt);
	if (d6.m128_f32[0] >= 0.0f && d5.m128_f32[0] <= d6.m128_f32[0])
	{
		*closest = triangle.p2;
		return;
	}

	// pointがp0_p2の辺領域の中にあるかどうかチェックし、あればpointのp0_p2上に対する射影を返す
	float vb = d5.m128_f32[0] * d2.m128_f32[0] - d1.m128_f32[0] * d6.m128_f32[0];
	if (vb <= 0.0f && d2.m128_f32[0] >= 0.0f && d6.m128_f32[0] <= 0.0f)
	{
		float w = d2.m128_f32[0] / (d2.m128_f32[0] - d6.m128_f32[0]);
		*closest = triangle.p0 + w * p0_p2;
		return;
	}

	// pointがp1_p2の辺領域の中にあるかどうかチェックし、あればpointのp1_p2上に対する射影を返す
	const float va = d3.m128_f32[0] * d6.m128_f32[0] - d5.m128_f32[0] * d4.m128_f32[0];
	if (va <= 0.0f && (d4.m128_f32[0] - d3.m128_f32[0]) >= 0.0f && (d5.m128_f32[0] - d6.m128_f32[0]) >= 0.0f)
	{
		const float w = (d4.m128_f32[0] - d3.m128_f32[0]) / ((d4.m128_f32[0] - d3.m128_f32[0]) + (d5.m128_f32[0] - d6.m128_f32[0]));
		*closest = triangle.p1 + w * (triangle.p2 - triangle.p1);
		return;
	}

	const float denom = 1.0f / (va + vb + vc);
	const float v = vb * denom;
	const float w = vc * denom;
	*closest = triangle.p0 + p0_p1 * v + p0_p2 * w;
}

bool Collision::CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle, DirectX::XMVECTOR* inter)
{
	XMVECTOR p{};
	// 球の中心に対する最近接点である三角形上にある点pを見つける
	ClosestPtPoint2Triangle(sphere.center, triangle, &p);
	// 点pと球の中心の差分ベクトル
	XMVECTOR v = p - sphere.center;
	// 距離の二乗を求める
	// (同じベクトル同士の内積は三平方の定理のルート内部の式と一致する)
	v = XMVector3Dot(v, v);
	// 球と三角形の距離が半径以下なら当たっていない
	if (v.m128_f32[0] > sphere.radius * sphere.radius) return false;
	// 疑似交点を計算
	if (inter)
	{
		// 三角形上の最近接点pを疑似交点とする
		*inter = p;
	}
	return true;
}

bool Collision::CheckRay2Plane(const Ray& ray, const Plane& plane, float* distance, DirectX::XMVECTOR* inter)
{
	constexpr float epsilon = 1.0e-5f;	//誤差吸収用の微小な値
	// 面方向とレイの方向ベクトルの内積
	const float d1 = XMVector3Dot(plane.normal, ray.dir).m128_f32[0];
	// 裏面には当たらない
	if (d1 > -epsilon) return false;
	// 始点と原点の距離(平面の法線方向)
	// 面方向とレイの始点座標(位置ベクトル)の内積
	const float d2 = XMVector3Dot(plane.normal, ray.start).m128_f32[0];
	// 始点と平面の距離(平面の法線方向)
	const float dist = d2 - plane.distance;
	// 始点と平面の距離(レイ方向)
	const float t = dist / -d1;
	// 交点が始点より後ろなら当たらない
	if (t < 0) return false;
	// 距離を書き込む
	if (distance) *distance = t;
	// 交点を計算
	if (inter) *inter = ray.start + t * ray.dir;

	return true;
}

bool Collision::CheckRay2Triangle(const Ray& ray, const Triangle& triangle, float* distance, DirectX::XMVECTOR* inter)
{
	//三角形が乗っている平面を算出
	Plane plane{};
	XMVECTOR interPlane{};
	plane.normal = triangle.normal;
	plane.distance = XMVector3Dot(triangle.normal, triangle.p0).m128_f32[0];
	//レイと平面が当たっていなければfalseを返す
	if (!CheckRay2Plane(ray, plane, distance, &interPlane)) return false;
	//-----当たっていたので、距離と交点が書き込まれた
	//交点が三角形の内側にあるか判定
	const float epsilon = 1.0e-5f;	//誤差吸収用(境界ぎりぎりを当たったとするため)
	XMVECTOR m{};
	//辺p0_p1について
	const XMVECTOR pt_p0 = triangle.p0 - interPlane;
	const XMVECTOR p0_p1 = triangle.p1 - triangle.p0;
	m = XMVector3Cross(pt_p0, p0_p1);
	//辺の外側なら当たっていないので判定終了
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) return false;
	//辺p1_p2について
	//辺の外側なら当たっていないので判定終了
	const XMVECTOR pt_p1 = triangle.p1 - interPlane;
	const XMVECTOR p1_p2 = triangle.p2 - triangle.p1;
	m = XMVector3Cross(pt_p1, p1_p2);
	//辺の外側なら当たっていないので判定終了
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) return false;
	//辺p2_p0について
	//辺の外側なら当たっていないので判定終了
	const XMVECTOR pt_p2 = triangle.p2 - interPlane;
	const XMVECTOR p2_p0 = triangle.p0 - triangle.p2;
	m = XMVector3Cross(pt_p2, p2_p0);
	//辺の外側なら当たっていないので判定終了
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) return false;
	//内側なので当たっている
	if (inter) *inter = interPlane;
	return true;
}

bool Collision::CheckRay2Sphere(const Ray& ray, const Sphere& sphere, float* distance, DirectX::XMVECTOR* inter)
{
	XMVECTOR m = ray.start - sphere.center;
	const float b = XMVector3Dot(m, ray.dir).m128_f32[0];
	const float c = XMVector3Dot(m, m).m128_f32[0] - sphere.radius * sphere.radius;
	// layの始点がsphereの外側にあり(c > 0)、layがsphereから離れる方向をさす場合(b > 0)、当たらない
	if (c > 0.f && b > 0.f) return false;

	const float discr = b * b - c;
	// 負の判別式はレイが球を離れていることに一致
	if (discr < 0.f) return false;

	// ----- レイは球と交差している

	// 交差する最小の値tを計算
	float t = -b - sqrtf(discr);
	// tが負である場合、レイは球の内側から開始しているのでtをゼロにクランプ
	if (t < 0) t = 0.f;
	if (distance) *distance = t;

	if (inter) *inter = ray.start + t * ray.dir;

	return true;
}

bool Collision::CheckSphere2AABB(const Sphere& sphere, const AABB& aabb)
{
	// 球の中心とAABBの距離の2乗を求める
	float sqDistance = 0.f;

	// x方向の距離の2乗を加算
	float pos = sphere.center.m128_f32[0];
	if (pos < aabb.minPos.m128_f32[0])
	{
		sqDistance += (sphere.center.m128_f32[0] - aabb.minPos.m128_f32[0])
			* (sphere.center.m128_f32[0] - aabb.minPos.m128_f32[0]);
	} else if (pos > aabb.maxPos.m128_f32[0])
	{
		sqDistance += (sphere.center.m128_f32[0] - aabb.maxPos.m128_f32[0])
			* (sphere.center.m128_f32[0] - aabb.maxPos.m128_f32[0]);
	}

	// y方向の距離の2乗を加算
	pos = sphere.center.m128_f32[1];
	if (pos < aabb.minPos.m128_f32[1])
	{
		sqDistance += (sphere.center.m128_f32[1] - aabb.minPos.m128_f32[1])
			* (sphere.center.m128_f32[1] - aabb.minPos.m128_f32[1]);
	} else if (pos > aabb.maxPos.m128_f32[1])
	{
		sqDistance += (sphere.center.m128_f32[1] - aabb.maxPos.m128_f32[1])
			* (sphere.center.m128_f32[1] - aabb.maxPos.m128_f32[1]);
	}

	// z方向の距離の2乗を加算
	pos = sphere.center.m128_f32[2];
	if (pos < aabb.minPos.m128_f32[2])
	{
		sqDistance += (sphere.center.m128_f32[2] - aabb.minPos.m128_f32[2])
			* (sphere.center.m128_f32[2] - aabb.minPos.m128_f32[2]);
	} else if (pos > aabb.maxPos.m128_f32[2])
	{
		sqDistance += (sphere.center.m128_f32[2] - aabb.maxPos.m128_f32[2])
			* (sphere.center.m128_f32[2] - aabb.maxPos.m128_f32[2]);
	}

	const float rr = sphere.radius * sphere.radius;
	return sqDistance < rr;
}

bool Collision::CheckSphere2Capsule(const Sphere& sphere, const Capsule& capsule)
{
	// 1. カプセル内の線分のスタート位置からエンド位置のベクトルを作る
	const XMVECTOR vStartToEnd = capsule.endPos - capsule.startPos;

	// 2. 1.のベクトルを単位ベクトル化し、nに用意する
	const XMVECTOR n = XMVector3Normalize(vStartToEnd);

	// 3. Ps -> Pc のベクトルと 2. で求めた n との内積を計算
	// (すると n を何倍すればベクトル Ps -> Pnになるか 倍率(t) が決まる)
	const float t = vec3Dot(sphere.center - capsule.startPos, n);

	// 4. Ps -> Pn ベクトルを求めておく、また、Pnの座標を求めておく
	const XMVECTOR vPsPn = n * t;
	const XMVECTOR posPn = vPsPn + capsule.startPos;

	// 5. 比率 t / (Ps -> Pe の長さ) を求める
	const float lengthRate = t / vecLength(vStartToEnd);

	// 6. lengthRate < 0, 0 <= lengthRate <= 1, 1 < lengthRate で場合分け
	float distance{};
	if (lengthRate < 0.f)
	{
		distance = vecLength(sphere.center - capsule.startPos) - capsule.radius;
	} else if (lengthRate <= 1.f)
	{
		distance = vecLength(sphere.center - posPn) - capsule.radius;
	} else
	{
		distance = vecLength(sphere.center - capsule.endPos) - capsule.radius;
	}

	return distance < sphere.radius;
}

bool Collision::CheckCapsule2Capsule(const Capsule& capsule1, const Capsule& capsule2)
{
	const float sqDistance = sqDistanceSegmentSegment(capsule1.startPos, capsule1.endPos,
													  capsule2.startPos, capsule2.endPos);

	const float rSum = capsule1.radius + capsule2.radius;

	return rSum * rSum > sqDistance;
}

bool Collision::CheckSphere2Sphere(const Sphere& sphere1, const Sphere& sphere2)
{
	const XMVECTOR len = sphere1.center - sphere2.center;

	const float sqLen =
		len.m128_f32[0] * len.m128_f32[0] +
		len.m128_f32[1] * len.m128_f32[1] +
		len.m128_f32[2] * len.m128_f32[2];

	const float rSum = sphere1.radius + sphere2.radius;

	return rSum * rSum > sqLen;
}