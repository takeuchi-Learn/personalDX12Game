#include "Camera.h"

using namespace DirectX;

DX12Base* Camera::dxBase = DX12Base::getInstance();

namespace
{
	XMFLOAT3 operator-(const XMFLOAT3& left, const XMFLOAT3& right)
	{
		return XMFLOAT3(
			left.x - right.x,
			left.y - right.y,
			left.z - right.z);
	}
	XMFLOAT3 operator+(const XMFLOAT3& left, const XMFLOAT3& right)
	{
		return XMFLOAT3(
			left.x + right.x,
			left.y + right.y,
			left.z + right.z);
	}

	XMFLOAT3 operator*(const XMFLOAT3& left, const float right)
	{
		return XMFLOAT3(
			left.x * right,
			left.y * right,
			left.z * right);
	}
}

void Camera::updateViewMatrix()
{
	// 視点座標
	XMVECTOR eyePosition = XMLoadFloat3(&eye);
	// 注視点座標
	XMVECTOR targetPosition = XMLoadFloat3(&target);
	// （仮の）上方向
	XMVECTOR upVector = XMLoadFloat3(&up);

	// カメラZ軸（視線方向）
	cameraAxisZ = XMVectorSubtract(targetPosition, eyePosition);
	// 0ベクトルだと向きが無い除外
	assert(!XMVector3Equal(cameraAxisZ, XMVectorZero()));
	assert(!XMVector3IsInfinite(cameraAxisZ));
	assert(!XMVector3Equal(upVector, XMVectorZero()));
	assert(!XMVector3IsInfinite(upVector));
	// ベクトルを正規化
	cameraAxisZ = XMVector3Normalize(cameraAxisZ);

	// カメラのX軸（右方向）
	XMVECTOR cameraAxisX;
	// X軸は上方向→Z軸の外積で求まる
	cameraAxisX = XMVector3Cross(upVector, cameraAxisZ);
	// ベクトルを正規化
	cameraAxisX = XMVector3Normalize(cameraAxisX);

	// カメラのY軸（上方向）
	XMVECTOR cameraAxisY;
	// Y軸はZ軸→X軸の外積で求まる
	cameraAxisY = XMVector3Cross(cameraAxisZ, cameraAxisX);

	// ここまでで直交した3方向のベクトルが揃う
	//（ワールド座標系でのカメラの右方向、上方向、前方向）

	// カメラ回転行列
	XMMATRIX matCameraRot{};
	// カメラ座標系→ワールド座標系の変換行列
	matCameraRot.r[0] = cameraAxisX;
	matCameraRot.r[1] = cameraAxisY;
	matCameraRot.r[2] = cameraAxisZ;
	matCameraRot.r[3] = XMVectorSet(0, 0, 0, 1);
	// 転置により逆行列（逆回転）を計算
	matView = XMMatrixTranspose(matCameraRot);

	// 視点座標に-1を掛けた座標
	XMVECTOR reverseEyePosition = XMVectorNegate(eyePosition);
	// カメラの位置からワールド原点へのベクトル（カメラ座標系）
	XMVECTOR tX = XMVector3Dot(cameraAxisX, reverseEyePosition);	// X成分
	XMVECTOR tY = XMVector3Dot(cameraAxisY, reverseEyePosition);	// Y成分
	XMVECTOR tZ = XMVector3Dot(cameraAxisZ, reverseEyePosition);	// Z成分
	// 一つのベクトルにまとめる
	XMVECTOR translation = XMVectorSet(tX.m128_f32[0], tY.m128_f32[1], tZ.m128_f32[2], 1.0f);
	// ビュー行列に平行移動成分を設定
	matView.r[3] = translation;

#pragma region 全方向ビルボード行列の計算
	// ビルボード行列
	matBillboard.r[0] = cameraAxisX;
	matBillboard.r[1] = cameraAxisY;
	matBillboard.r[2] = cameraAxisZ;
	matBillboard.r[3] = XMVectorSet(0, 0, 0, 1);
#pragma region

#pragma region Y軸回りビルボード行列の計算
	// カメラX軸、Y軸、Z軸
	XMVECTOR ybillCameraAxisX, ybillCameraAxisY, ybillCameraAxisZ;

	// X軸は共通
	ybillCameraAxisX = cameraAxisX;
	// Y軸はワールド座標系のY軸
	ybillCameraAxisY = XMVector3Normalize(upVector);
	// Z軸はX軸→Y軸の外積で求まる
	ybillCameraAxisZ = XMVector3Cross(ybillCameraAxisX, ybillCameraAxisY);

	// Y軸回りビルボード行列
	matBillboardY.r[0] = ybillCameraAxisX;
	matBillboardY.r[1] = ybillCameraAxisY;
	matBillboardY.r[2] = ybillCameraAxisZ;
	matBillboardY.r[3] = XMVectorSet(0, 0, 0, 1);
#pragma endregion
}

void Camera::updateProjectionMatrix()
{
	// 透視投影による射影行列の生成
	matProjection = XMMatrixPerspectiveFovLH(
		fogAngleYRad,
		aspectRatio,
		nearZ, farZ
	);
}

void Camera::updateMatrix()
{
	if (viewDirty || projectionDirty)
	{
		// 再計算必要なら
		if (viewDirty)
		{
			// ビュー行列更新
			updateViewMatrix();
			viewDirty = false;
		}

		// 再計算必要なら
		if (projectionDirty)
		{
			// ビュー行列更新
			updateProjectionMatrix();
			projectionDirty = false;
		}
		// ビュープロジェクションの合成
		matViewProjection = matView * matProjection;
		matVPV = matViewProjection * matViewPort;
	}
}

void Camera::moveEye(const XMFLOAT3& move)
{
	// 視点座標を移動し、反映
	XMFLOAT3 eye_moved = getEye();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	setEye(eye_moved);
}

void Camera::moveEye(const XMVECTOR& move)
{
	// 視点座標を移動し、反映
	XMFLOAT3 eye_moved = getEye();

	eye_moved.x += move.m128_f32[0];
	eye_moved.y += move.m128_f32[1];
	eye_moved.z += move.m128_f32[2];

	setEye(eye_moved);
}

void Camera::moveCamera(const XMFLOAT3& move)
{
	// 視点と注視点座標を移動し、反映
	XMFLOAT3 eye_moved = getEye();
	XMFLOAT3 target_moved = getTarget();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	target_moved.x += move.x;
	target_moved.y += move.y;
	target_moved.z += move.z;

	setEye(eye_moved);
	setTarget(target_moved);
}

void Camera::moveCamera(const XMVECTOR& move)
{
	// 視点と注視点座標を移動し、反映
	XMFLOAT3 eye_moved = getEye();
	XMFLOAT3 target_moved = getTarget();

	eye_moved.x += move.m128_f32[0];
	eye_moved.y += move.m128_f32[1];
	eye_moved.z += move.m128_f32[2];

	target_moved.x += move.m128_f32[0];
	target_moved.y += move.m128_f32[1];
	target_moved.z += move.m128_f32[2];

	setEye(eye_moved);
	setTarget(target_moved);
}

Camera::Camera(const float window_width, const float window_height)
{
	fogAngleYRad = DirectX::XM_PI / 3.f;

	aspectRatio = window_width / window_height;

	//ビュー行列の計算
	updateViewMatrix();

	// 射影行列の計算
	updateProjectionMatrix();

	// ビュープロジェクションの合成
	matViewProjection = matView * matProjection;

	matViewPort = XMMatrixIdentity();
	matViewPort.r[0].m128_f32[0] = (float)window_width / 2.f;
	matViewPort.r[1].m128_f32[1] = -1.f - ((float)window_height / 2.f);
	matViewPort.r[3].m128_f32[0] = matViewPort.r[0].m128_f32[0];
	matViewPort.r[3].m128_f32[1] = (float)window_height / 2.f;
}

Camera::~Camera()
{
#ifdef _DEBUG
	OutputDebugStringA("delete camera\n");
#endif // _DEBUG
}

void Camera::update()
{
	preUpdate();
	updateMatrix();
	postUpdate();
}

XMFLOAT3 Camera::screenPos2WorldPos(const XMFLOAT3& screenPos) const
{
	// 照準が指し示す3Dの座標
	XMFLOAT3 wpos{};
	XMStoreFloat3(&wpos, screenPos2WorldPosVec(screenPos));

	return wpos;
}

XMVECTOR Camera::screenPos2WorldPosVec(const XMFLOAT3& screenPos) const
{
	const XMMATRIX matVPVInv = XMMatrixInverse(nullptr, matVPV);

	// ワールド座標
	const XMVECTOR nearPos = XMVector3TransformCoord(XMVectorSet(screenPos.x, screenPos.y, 0.f, 0.f), matVPVInv);
	const XMVECTOR farPos = XMVector3TransformCoord(XMVectorSet(screenPos.x, screenPos.y, 1.f, 0.f), matVPVInv);

	// マウスが指し示すベクトル
	const XMVECTOR mouseDir = screenPos.z * XMVector3Normalize(farPos - nearPos);

	// 照準が指し示す3Dの座標
	return nearPos + mouseDir;
}

XMFLOAT3 Camera::calcLook() const
{
	// 視線ベクトル
	XMFLOAT3 look = target - eye;
	// XMVECTORを経由して正規化
	const XMVECTOR normalLookVec = XMVector3Normalize(XMLoadFloat3(&look));
	//XMFLOAT3に戻す
	XMStoreFloat3(&look, normalLookVec);

	return look;
}

void Camera::rotation(const float targetlength, const float angleX, const float angleY)
{
	// 視線ベクトル
	const XMFLOAT3& look = calcLook();

	constexpr float lookLen = 50.f;
	XMFLOAT3 newTarget = eye;
	newTarget.x += targetlength * dxBase->nearSin(angleY) * dxBase->nearCos(angleX) + look.x * lookLen;
	newTarget.y += targetlength * dxBase->nearSin(angleX) + look.y * lookLen;
	newTarget.z += targetlength * dxBase->nearCos(angleY) * dxBase->nearCos(angleX) + look.z * lookLen;

	setTarget(newTarget);
}

void Camera::moveForward(const float speed)
{
	const XMFLOAT3 moveVal = calcLook() * speed;

	moveCamera(moveVal);
}

void Camera::moveRight(const float speed)
{
	const XMFLOAT3 moveVal = calcLook() * speed;

	const XMFLOAT3 val{ moveVal.z, /*moveVal.y*/0, -moveVal.x };

	moveCamera(val);
}