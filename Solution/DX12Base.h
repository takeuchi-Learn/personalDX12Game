#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>	//ComPtr

#include "WinAPI.h"

#include <DirectXMath.h>

#include <imgui.h>

class DX12Base {
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
	static const USHORT divNum = 8;
	LONGLONG fpsTime[divNum]{};
	float fps;

	void flipTimeFPS();
	void updateFPS();

public:
	float getFPS();

private:

#pragma endregion


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
	static DX12Base* getInstance();

	// @param clearColor 何もない場所の描画色。既定引数は暗い黄色っぽい色
	void startDraw(const DirectX::XMFLOAT3& clearColor = DirectX::XMFLOAT3(0.5f, 0.5f, 0.f));
	void endDraw();

	ID3D12Device* getDev();
	ID3D12GraphicsCommandList* getCmdList();
};

