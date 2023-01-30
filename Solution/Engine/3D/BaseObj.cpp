#include "BaseObj.h"

using namespace DirectX;

void BaseObj::updateMatWorld()
{
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	matWorld = XMMatrixIdentity();
	matWorld *= matScale;
	matWorld *= matRot;
	if (isBillboard)
	{
		matWorld *= camera->getBillboardMatrix();
	} else if (isBillBoardY)
	{
		matWorld *= camera->getBillboardMatrixY();
	}
	matWorld *= matTrans; // ワールド行列に平行移動を反映

	// 親オブジェクトがあれば
	if (parent)
	{
		// 親オブジェクトのワールド行列を掛ける
		matWorld *= parent->getMatWorld();
	}
}

BaseObj::BaseObj(Camera* camera) :
	camera(camera)
{}

XMFLOAT2 BaseObj::calcScreenPos()
{
	XMVECTOR screenPosVec = XMVector3Transform(matWorld.r[3],
											   camera->getMatVPV());
	screenPosVec /= XMVectorGetW(screenPosVec);
	XMFLOAT2 screenPosF2{};
	XMStoreFloat2(&screenPosF2, screenPosVec);

	return screenPosF2;
}