#pragma once

#include <DirectXMath.h>

class Camera {
private:
	// エイリアス
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;

private:
	// ビュー行列
	XMMATRIX matView = DirectX::XMMatrixIdentity();
	// ビルボード行列
	XMMATRIX matBillboard = DirectX::XMMatrixIdentity();
	// Y軸回りビルボード行列
	XMMATRIX matBillboardY = DirectX::XMMatrixIdentity();
	// 射影行列
	XMMATRIX matProjection = DirectX::XMMatrixIdentity();
	// ビュー射影行列
	XMMATRIX matViewProjection = DirectX::XMMatrixIdentity();
	// ビュー行列ダーティフラグ
	bool viewDirty = false;
	// 射影行列ダーティフラグ
	bool projectionDirty = false;
	// 視点座標
	XMFLOAT3 eye = { 0, 0, -20 };
	// 注視点座標
	XMFLOAT3 target = { 0, 0, 0 };
	// 上方向ベクトル
	XMFLOAT3 up = { 0, 1, 0 };
	// アスペクト比
	float aspectRatio = 1.0f;

	float nearZ = 0.1f;
	float farZ = 1000.f;
	float fogAngleYRad = DirectX::XM_PI / 3.f;

	// --------------------
	// メンバ関数
	// --------------------
private:

	// ビュー行列を更新
	void updateViewMatrix();

	// 射影行列を更新
	void updateProjectionMatrix();

public:
	// ビュー行列の取得
	// @return matView
	inline const XMMATRIX& getViewMatrix() { return matView; }

	// 射影行列の取得
	// @return matProjection
	inline const XMMATRIX& getProjectionMatrix() { return matProjection; }

	// ビュー射影行列の取得
	inline const XMMATRIX& getViewProjectionMatrix() { return matViewProjection; }

	// ビルボード行列の取得
	inline const XMMATRIX& getBillboardMatrix() { return matBillboard; }

	// Y軸固定ビルボード行列の取得
	inline const XMMATRIX& getBillboardMatrixY() { return matBillboardY; }

	// 視点座標の取得
	inline const XMFLOAT3& getEye() { return eye; }

	// 視点座標の設定
	inline void setEye(const XMFLOAT3& eye) { this->eye = eye; viewDirty = true; }

	// 注視点座標の取得
	inline const XMFLOAT3& getTarget() { return target; }

	// 注視点座標の設定
	inline void setTarget(const XMFLOAT3& target) { this->target = target; viewDirty = true; }

	// 上方向ベクトルの取得
	inline const XMFLOAT3& getUp() { return up; }
	// 上方向ベクトルの設定
	inline void setUp(const XMFLOAT3& up) { this->up = up; viewDirty = true; }

	inline void setNearZ(const float nearZ) { this->nearZ = nearZ; projectionDirty = true; }
	inline void setFarZ(const float farZ) { this->farZ = farZ; projectionDirty = true; }

	inline void setFogAngleYRad(const float fogAngleYRad) { this->fogAngleYRad = fogAngleYRad; projectionDirty = true; }

	inline float getNearZ() { return nearZ; }
	inline float getFarZ() { return farZ; }

	inline float getFogAngleYRad() { return fogAngleYRad; }

	const XMFLOAT3 &getLook() const;

	/// <summary>
	/// カメラを回転
	/// </summary>
	/// <param name="targetlength">カメラから注視点までの距離</param>
	/// <param name="angleX">X軸周りの回転角(-PI/2 ~ PI/2の範囲で送る)</param>
	/// <param name="angleY">Y軸周りの回転角(0 ~ 2PIの範囲で送る)</param>
	void rotation(const float targetlength,
						const float angleX, const float angleY);

	void moveForward(const float speed);

	void moveRight(const float speed);


	/// <summary>
	/// ベクトルによる視点移動(eyeのみの移動、targetは変わらない)
	/// </summary>
	/// <param name="move">移動量</param>
	void moveEye(const XMFLOAT3& move);
	void moveEye(const XMVECTOR& move);

	/// <summary>
	/// ベクトルによる移動(eyeとtargetを移動)
	/// </summary>
	/// <param name="move">移動量</param>
	void moveCamera(const XMFLOAT3& move);
	void moveCamera(const XMVECTOR& move);

	Camera(const float window_width, const float window_height);

	~Camera();

	void update();
};

