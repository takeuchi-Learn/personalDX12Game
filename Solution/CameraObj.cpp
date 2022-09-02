﻿#include "CameraObj.h"

using namespace DirectX;

CameraObj::CameraObj(GameObj *parent)
	:Camera(WinAPI::window_width,
			WinAPI::window_height),
	parentObj(parent) {
	relativeRotaDeg = XMFLOAT3(30, 0, 0);
}

void CameraObj::preUpdate() {
	if (parentObj == nullptr) return;

	// カメラ操作を反転するかどうか
	constexpr bool invCamOperFlag = false;
	// 反転するなら1、しないなら-1
	constexpr float rotaInvFactor = invCamOperFlag ? 1.f : -1.f;


	XMFLOAT3 targetPos = parentObj->getPos();

	// 親の回転を取得[rad]
	const XMFLOAT3 rotaAngleRad{
		XMConvertToRadians(rotaInvFactor * (parentObj->getRotation().x + relativeRotaDeg.x)),
		XMConvertToRadians(rotaInvFactor * (parentObj->getRotation().y + relativeRotaDeg.y)),
		XMConvertToRadians(rotaInvFactor * (parentObj->getRotation().z + relativeRotaDeg.z))
	};

	// 垂直角度を計算
	float sinT = dxBase->nearSin(-rotaAngleRad.x);
	float cosT = dxBase->nearCos(-rotaAngleRad.x);

	const XMFLOAT3 verticalPos{
		0.f,
		sinT * eye2TargetLen,
		-cosT * eye2TargetLen
	};

	// 水平角度を計算
	sinT = dxBase->nearSin(rotaAngleRad.y);
	cosT = dxBase->nearCos(rotaAngleRad.y);
	const XMFLOAT3 horizontalPos{
		cosT * verticalPos.x - sinT * verticalPos.z,
		verticalPos.y,
		sinT * verticalPos.x + cosT * verticalPos.z
	};

	// 計算した座標 + 注視点の位置 = カメラの位置
	XMFLOAT3 eye{
		horizontalPos.x + targetPos.x,
		horizontalPos.y + targetPos.y,
		horizontalPos.z + targetPos.z,
	};

	// 前回のカメラ位置
	const XMFLOAT3 oldEye = getEye();

	// 補間割合を加味したカメラ移動幅
	constexpr float raito = 0.1f;
	eye = {
		(eye.x - oldEye.x) * raito,
		(eye.y - oldEye.y) * raito,
		(eye.z - oldEye.z) * raito
	};

	// 前回の位置に移動幅を足す
	eye = {
		oldEye.x + eye.x,
		oldEye.y + eye.y,
		oldEye.z + eye.z
	};


	// 注視点の位置を高くする
	{

		XMFLOAT3 camHeiVec{};
		XMStoreFloat3(&camHeiVec, XMVector3Transform(XMVectorSet(eye2TargetOffset.x,
																 eye2TargetOffset.y,
																 eye2TargetOffset.z,
																 1),
													 parentObj->getObj()->getMatRota()));
		targetPos.x += camHeiVec.x;
		targetPos.y += camHeiVec.y;
		targetPos.z += camHeiVec.z;
	}


	setEye(eye);
	setTarget(targetPos);
	//setUp(XMFLOAT3(0, 1, 0));
}