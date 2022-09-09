#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>	//ComPtr

#include "WinAPI.h"

#include <DirectXMath.h>

#include <imgui.h>

class DX12Base
{
	DX12Base(const DX12Base& dxBase) = delete;
	DX12Base& operator=(const DX12Base& dxBase) = delete;

	DX12Base();
	~DX12Base();

#pragma region privateメンバ変数

	Microsoft::WRL::ComPtr<ID3D12Device> dev;
	Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeaps;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> imguiHeap;

	WinAPI* winapi = nullptr;

#pragma endregion

#pragma region FPS
private:
	static const USHORT divNum = 2;	// 2以上にする
	LONGLONG fpsTime[divNum]{};
	float fps;

	void flipTimeFPS();
	void updateFPS();

public:
	inline float getFPS() const { return fps; }

#pragma endregion

private:

	void initDevice();
	void initCommand();
	void initSwapchain();
	void initRTV();
	void initDepthBuffer();
	void initFence();

	/// <summary>
	/// imgui初期化
	/// </summary>
	/// <returns>成否</returns>
	bool InitImgui();

	// 全画面クリア
	void ClearRenderTarget(const DirectX::XMFLOAT3& clearColor);
	// 深度バッファクリア
	void ClearDepthBuffer();

public:
	inline static DX12Base* getInstance()
	{
		static DX12Base dxBase{};
		return &dxBase;
	}

#pragma region 角度系

	// @return 0 <= ret[rad] < 2PI
	float angleRoundRad(float rad);

	float nearSin(float rad);

	float nearCos(float rad);

	float nearTan(float rad);

	double near_atan2(double _y, double _x);

	float near_atan2(float y, float x);

	float nearAcos(float x);

#pragma endregion

	// @param clearColor 何もない場所の描画色。既定引数は暗い黄色っぽい色
	void startDraw(const DirectX::XMFLOAT3& clearColor = DirectX::XMFLOAT3(0.5f, 0.5f, 0.f));
	void endDraw();

	inline ID3D12Device* getDev() { return dev.Get(); };
	inline ID3D12GraphicsCommandList* getCmdList() { return cmdList.Get(); };
};
