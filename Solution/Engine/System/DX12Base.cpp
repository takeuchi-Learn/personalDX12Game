#include "DX12Base.h"

#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

#include <chrono>

#include <DirectXMath.h>

#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

#include <DirectXMath.h>

#include <Util/Timer.h>
#include <ImGuiData/KaisoTai_base85.h>
#include <ImGuiData/Makinas_4_Flat_base85.h>
#include <ImGuiData/GlyphRangesJapanese.h>

using namespace DirectX;

#pragma region 角度系関数

float DX12Base::angleRoundRad(float rad)
{
	float angle = rad;

	if (angle >= 0.f && angle < XM_2PI) return angle;

	while (angle >= XM_2PI)
	{
		angle -= XM_2PI;
	}
	while (angle < 0)
	{
		angle += XM_2PI;
	}
	return angle;
}

float DX12Base::nearSin(float rad)
{
	constexpr float a = +0.005859483f;
	constexpr float b = +0.005587939f;
	constexpr float c = -0.171570726f;
	constexpr float d = +0.0018185485f;
	constexpr float e = +0.9997773594f;

	float x = angleRoundRad(rad);

	// 0 ~ PI/2がわかれば求められる
	if (x < XM_PIDIV2)
	{
		// そのまま
	} else if (x >= XM_PIDIV2 && x < XM_PI)
	{
		x = XM_PI - x;
	} else if (x < XM_PI * 1.5f)
	{
		x = -(x - XM_PI);
	} else if (x < XM_2PI)
	{
		x = -(XM_2PI - x);
	}

	return x * (x * (x * (x * (a * x + b) + c) + d) + e);
}

float DX12Base::nearCos(float rad)
{
	return nearSin(rad + XM_PIDIV2);
}

float DX12Base::nearTan(float rad)
{
	return nearSin(rad) / nearCos(rad);
}

double DX12Base::near_atan2(double _y, double _x)
{
	const double x = abs(_x);
	const double y = abs(_y);

	const bool bigX = y < x;

	double slope{};
	if (bigX) slope = (double)y / x;
	else  slope = (double)x / y;

	constexpr double a = -0.05026472;
	constexpr double b = +0.26603324;
	constexpr double c = -0.45255286;
	constexpr double d = +0.02385002;
	constexpr double e = +0.99836359;

	double ret = slope * (slope * (slope * (slope * (a * slope + b) + c) + d) + e); //5次曲線近似

	constexpr float plane = XM_PI;
	constexpr float rightAngle = plane / 2.f;	// 直角

	if (bigX)
	{
		if (_x > 0)
		{
			if (_y < 0) ret = -ret;
		} else
		{
			if (_y > 0) ret = plane - ret;
			if (_y < 0) ret = ret - plane;
		}
	} else
	{
		if (_x > 0)
		{
			if (_y > 0) ret = rightAngle - ret;
			if (_y < 0) ret = ret - rightAngle;
		}
		if (_x < 0)
		{
			if (_y > 0) ret = ret + rightAngle;
			if (_y < 0) ret = -ret - rightAngle;
		}
	}

	return ret;
}

float DX12Base::near_atan2(float y, float x)
{
	return (float)near_atan2((double)y, (double)x);
}

float DX12Base::nearAcos(float x)
{
	const float negate = x < 0 ? 1.f : 0.f;
	x = abs(x);
	float ret = -0.0187293f;
	ret = ret * x;
	ret = ret + 0.0742610f;
	ret = ret * x;
	ret = ret - 0.2121144f;
	ret = ret * x;
	ret = ret + 1.5707288f;
	ret = ret * sqrtf(1.f - x);
	ret = ret * (1.f - 2.f * negate);
	return negate * XM_PI + ret;
}

#pragma endregion 角度系関数

void DX12Base::initDevice()
{
	//DXGiファクトリ(デバイス生成後は解放されてよい)
	//ComPtr<IDXGIFactory6> dxgiFactory;

#ifdef _DEBUG
	////デバッグレイヤーをオンに
	//ComPtr<ID3D12Debug1> debugController;
	//if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	//{
	//	debugController->EnableDebugLayer();
	//	debugController->SetEnableGPUBasedValidation(TRUE);
	//}
#endif

	// アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter1>> adapters;
	// DXGIファクトリーの生成
	HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	// ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter1> tmpAdapter = nullptr;
	for (UINT i = 0;
		 dxgiFactory->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		 ++i)
	{
		adapters.push_back(tmpAdapter); // 動的配列に追加する
	}

	for (UINT i = 0, len = (UINT)adapters.size(); i < len; ++i)
	{
		DXGI_ADAPTER_DESC1 adesc;
		adapters[i]->GetDesc1(&adesc);  // アダプターの情報を取得

		// ソフトウェアデバイスを回避
		if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		std::wstring strDesc = adesc.Description;   // アダプター名
		// Intel UHD Graphics（オンボードグラフィック）を回避
		if (strDesc.find(L"Intel") == std::wstring::npos)
		{
			tmpAdapter = adapters[i];   // 採用
			break;
		}
	}

	// 対応レベルの配列
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (UINT i = 0, len = _countof(levels); i < len; ++i)
	{
		// 採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			// デバイスを生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(dev->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でDXGIとDX12のデバッグレイヤーの相互作用によるバグのエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		//抑制表示レベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したエラー表示抑制
		infoQueue->PushStorageFilter(&filter);
	}
#endif // _DEBUG
}

void DX12Base::initCommand()
{
	// コマンドアロケータを生成
	HRESULT result = dev->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));

	// コマンドリストを生成
	result = dev->CreateCommandList(0,
									D3D12_COMMAND_LIST_TYPE_DIRECT,
									cmdAllocator.Get(), nullptr,
									IID_PPV_ARGS(&cmdList));

	// 標準設定でコマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};

	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));
}

void DX12Base::initSwapchain()
{
	// 各種設定をしてスワップチェーンを生成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = WinAPI::window_width;
	swapchainDesc.Height = WinAPI::window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 色情報の書式
	swapchainDesc.SampleDesc.Count = 1; // マルチサンプルしない
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // バックバッファ用
	swapchainDesc.BufferCount = 2;  // バッファ数を２つに設定
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フリップ後は破棄
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swapchain1;

	// スワップチェーンの生成
	dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue.Get(),
		winapi->getHwnd(),
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapchain1);

	// 生成したIDXGISwapChain1のオブジェクトをIDXGISwapChain4に変換する
	swapchain1.As(&swapchain);
}

void DX12Base::initRTV()
{
	// 各種設定をしてデスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type =
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー
	heapDesc.NumDescriptors = 2;    // 裏表の２つ
	dev->CreateDescriptorHeap(&heapDesc,
							  IID_PPV_ARGS(&rtvHeaps));
	// 裏表の２つ分について
	constexpr UINT backBuffNum = 2u;
	backBuffers.resize(backBuffNum);

	HRESULT result = S_FALSE;

	for (UINT i = 0; i < backBuffNum; ++i)
	{
		// スワップチェーンからバッファを取得
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));

		// レンダーターゲットビューの生成
		dev->CreateRenderTargetView(
			backBuffers[i].Get(),
			nullptr,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
				rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
				i,
				dev->GetDescriptorHandleIncrementSize(heapDesc.Type)
			)
		);
	}
}

void DX12Base::initDepthBuffer()
{
	// 深度バッファリソース設定
	CD3DX12_RESOURCE_DESC depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		WinAPI::window_width,
		WinAPI::window_height,
		1, 0,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	// 深度バッファの生成
	HRESULT result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値書き込みに使用
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		IID_PPV_ARGS(&depthBuffer));

	// 深度ビュー用デスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1; // 深度ビューは1つ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // デプスステンシルビュー

	result = dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	// 深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // 深度値フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(
		depthBuffer.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void DX12Base::initFence()
{
	// フェンスの生成
	HRESULT result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
}

bool DX12Base::InitImgui()
{
	HRESULT result = S_FALSE;

	// デスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	result = dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&imguiHeap));
	if (FAILED(result))
	{
		assert(0);
		return false;
	}

	// スワップチェーンの情報を取得
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = swapchain->GetDesc(&swcDesc);
	if (FAILED(result))
	{
		assert(0);
		return false;
	}

	if (ImGui::CreateContext() == nullptr)
	{
		assert(0);
		return false;
	}
	if (!ImGui_ImplWin32_Init(WinAPI::getInstance()->getHwnd()))
	{
		assert(0);
		return false;
	}
	if (!ImGui_ImplDX12_Init(
		getDev(),
		swcDesc.BufferCount,
		swcDesc.BufferDesc.Format,
		imguiHeap.Get(),
		imguiHeap->GetCPUDescriptorHandleForHeapStart(),
		imguiHeap->GetGPUDescriptorHandleForHeapStart()
	))
	{
		assert(0);
		return false;
	}

	// フォントを使用
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;	// iniファイルを生成しない

	// 未指定なら最初に読み込んだフォントが使われる(と思われる)

	defaultImFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(Makinas_4_Flat_base85_compressed_data,
																   18.f,
																   nullptr,
																   glyphRangesJapanese);
	bigImFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(KaisoTai_base85_compressed_data_base85,
															   36.f,
															   nullptr,
															   glyphRangesJapanese);

	// 四角くする
	ImGuiStyle& st = ImGui::GetStyle();
	st.WindowRounding = 0.f;
	st.ItemSpacing = ImVec2(0.f, 0.f);
	st.DisplayWindowPadding = ImVec2(0.f, 0.f);

	return true;
}

void DX12Base::ClearRenderTarget(const DirectX::XMFLOAT3& clearColor)
{
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();

	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(), bbIndex, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	// 全画面クリア
	float clearColorTmp[] = { clearColor.x, clearColor.y, clearColor.z, 0.0f };
	cmdList->ClearRenderTargetView(rtvH, clearColorTmp, 0, nullptr);
}

void DX12Base::ClearDepthBuffer()
{
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	// 深度バッファのクリア
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

DX12Base::DX12Base() :
	winapi(WinAPI::getInstance()),
	fps(-1.f)
{
	updateFPS();
	flipTimeFPS();

	initDevice();
	initCommand();
	initSwapchain();
	initRTV();
	initDepthBuffer();
	initFence();

	// imgui初期化
	if (!InitImgui())
	{
		assert(0);
	}
}

DX12Base::~DX12Base()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void DX12Base::startDraw()
{
	// バックバッファの番号を取得（2つなので0番か1番）
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();

	// １．リソースバリアで書き込み可能に変更
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
																	  D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// ２．描画先指定
			// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
									  bbIndex, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		);
	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart()
		);
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	// ビューポート領域の設定
	cmdList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, WinAPI::window_width, WinAPI::window_height));
	// シザー矩形の設定
	cmdList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, WinAPI::window_width, WinAPI::window_height));
}

void DX12Base::endDraw()
{
	endResourceBarrier();

	// 命令のクローズ
	cmdList->Close();
	// コマンドリストの実行
	ID3D12CommandList* cmdLists[] = { cmdList.Get() }; // コマンドリストの配列
	cmdQueue->ExecuteCommandLists(1, cmdLists);

	// バッファをフリップ（裏表の入替え）
	swapchain->Present(1, 0);

	// コマンドキューの実行完了を待つ
	cmdQueue->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	cmdAllocator->Reset(); // キューをクリア
	cmdList->Reset(cmdAllocator.Get(), nullptr);  // 再びコマンドリストを貯める準備

	updateFPS();
	flipTimeFPS();
}

void DX12Base::clearBuffer(const DirectX::XMFLOAT3& clearColor)
{
	// 全画面クリア
	ClearRenderTarget(clearColor);
	// 深度バッファクリア
	ClearDepthBuffer();
}

void DX12Base::endResourceBarrier()
{
	// バックバッファの番号を取得（2つなので0番か1番）
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();

	// ５．リソースバリアを戻す
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIndex].Get(),
																	  D3D12_RESOURCE_STATE_RENDER_TARGET,
																	  D3D12_RESOURCE_STATE_PRESENT));
}

void DX12Base::startImGui()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DX12Base::endImGui()
{
	ImGui::Render();
	ID3D12DescriptorHeap* ppHeaps[] = { imguiHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());
}

// --------------------
// FPS
// --------------------

void DX12Base::flipTimeFPS()
{
	for (UINT i = divLen; i > 0; --i)
	{
		fpsTime[i] = fpsTime[i - 1];
	}
	fpsTime[0] = std::chrono::duration_cast<Timer::timeUnit>(
		std::chrono::steady_clock::now() - std::chrono::steady_clock::time_point()
	).count();
}

void DX12Base::updateFPS()
{
	LONGLONG avgDiffTime = 0ll;

	for (UINT i = 0; i < divLen; ++i)
	{
		avgDiffTime += fpsTime[i] - fpsTime[i + 1];
	}
	avgDiffTime /= divLen;

	fps = -1.f;
	if (avgDiffTime != 0ll) fps = Timer::oneSecF / avgDiffTime;
}