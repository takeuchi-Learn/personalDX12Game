#include "CameraObj.h"

using namespace DirectX;

CameraObj::CameraObj(GameObj *parent)
	:Camera(WinAPI::window_width,
			WinAPI::window_height),
	parentObj(parent) {
	relativePos = XMFLOAT3(0, 5, -10);
	relativeRotaDeg = XMFLOAT3(30, 0, 0);
}

void CameraObj::preUpdate() {
	// カメラ操作を反転するかどうか
	constexpr bool invCamOperFlag = false;
	// 反転するなら1、しないなら-1
	constexpr float rotaInvFactor = invCamOperFlag ? 1.f : -1.f;

	// 追従対象基準のカメラの高さ
	constexpr float obj2Targetheight = 60.f;
	// 視点から注視点までの距離
	constexpr float eye2TargetLen = 300.f;

	XMFLOAT3 targetPos = parentObj->getPos();
	targetPos.y += obj2Targetheight;

	// 親の回転を取得[rad]
	const XMFLOAT3 parentAngleRad{
		XMConvertToRadians(rotaInvFactor * parentObj->getRotation().x),
		XMConvertToRadians(rotaInvFactor * parentObj->getRotation().y),
		XMConvertToRadians(rotaInvFactor * parentObj->getRotation().z)
	};

	// 垂直角度を計算
	float sinT = dxBase->nearSin(parentAngleRad.x);
	float cosT = dxBase->nearCos(parentAngleRad.x);

	const XMFLOAT3 verticalPos{
		0.f,
		sinT * eye2TargetLen,
		-cosT * eye2TargetLen
	};

	// 垂直角度を計算
	sinT = dxBase->nearSin(parentAngleRad.y);
	cosT = dxBase->nearCos(parentAngleRad.y);
	const XMFLOAT3 horizontalPos{
		cosT * verticalPos.x - sinT * verticalPos.z,
		verticalPos.y,
		sinT * verticalPos.x + cosT * verticalPos.z
	};

	// 計算した座標 + 注視点の位置 = カメラの位置
	const XMFLOAT3 eye{
		horizontalPos.x + targetPos.x,
		horizontalPos.y + targetPos.y,
		horizontalPos.z + targetPos.z,
	};

	setEye(eye);
	setTarget(targetPos);
	setUp(XMFLOAT3(0, 1, 0));
}