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

	ImFont* defaultImFont = nullptr;
	ImFont* bigImFont = nullptr;

#pragma endregion

public:
	static const ImGuiWindowFlags imGuiWinFlagsDef =
		// リサイズ不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		// 設定を.iniに出力しない
		ImGuiWindowFlags_::ImGuiWindowFlags_NoSavedSettings |
		// 移動不可
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove;

	static const ImGuiWindowFlags ImGuiWinFlagsNoTitleBar = DX12Base::imGuiWinFlagsDef | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar;

#pragma region FPS
private:
	static const USHORT divNum = 2;	// 2以上にする
	LONGLONG fpsTime[divNum]{};
	constexpr static USHORT divLen = divNum - 1;
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

	/// @brief ImGui初期化
	/// @return 成否
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

	// getInstance()と同義
	inline static DX12Base* ins() { return getInstance(); }

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

	void startDraw();
	void endDraw();

	// @param clearColor 何もない場所の描画色。既定引数は暗い黄色っぽい色
	void clearBuffer(const DirectX::XMFLOAT3& clearColor = DirectX::XMFLOAT3(0.5f, 0.5f, 0.f));
	void endResourceBarrier();

	void startImGui();
	void endImGui();

	inline ID3D12Device* getDev() { return dev.Get(); };
	inline ID3D12GraphicsCommandList* getCmdList() { return cmdList.Get(); };

	inline ImFont* getBigImFont() const { return bigImFont; }
};
