#include "PostEffect.h"
#include <d3dx12.h>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

const float PostEffect::clearColor[4] = { 0.f, 0.f, 0.f, 1.f };

PostEffect::PostEffect()
	: mosaicNum({ WinAPI::window_width, WinAPI::window_height }),
	nowPPSet(0u),
	vignIntensity(0.25f),
	noiseIntensity(0.f),
	alpha(1.f),
	rgbShiftNum({ 0.f,0.f }),
	speedLineIntensity(0.f),
	timer(new Timer()),
	dev(DX12Base::getInstance()->getDev()),
	cmdList(DX12Base::getInstance()->getCmdList())
{
	pipelineSet.emplace_back();
	init();
}

void PostEffect::transferConstBuff(float nowTime, float oneSec)
{
	// 定数バッファにデータ転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->oneSec = oneSec;
	constMap->nowTime = nowTime;
	constMap->winSize = { (float)WinAPI::window_width, (float)WinAPI::window_height };
	constMap->noiseIntensity = noiseIntensity;
	constMap->mosaicNum = mosaicNum;
	constMap->vignIntensity = vignIntensity;
	constMap->alpha = alpha;
	constMap->rgbShiftNum = rgbShiftNum;
	constMap->speedLineIntensity = speedLineIntensity;
	constBuff->Unmap(0, nullptr);

	assert(SUCCEEDED(result));
}

void PostEffect::initBuffer()
{
	constexpr UINT vertNum = 4;

	// 頂点バッファ生成
	HRESULT result = DX12Base::getInstance()->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPosUv) * vertNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));

	constexpr DirectX::XMFLOAT2 drawSizeRaito = { 1.f, 1.f };
	VertexPosUv vertices[vertNum]{
		{{ -drawSizeRaito.x, -drawSizeRaito.y, 0.f }, { 0.f, 1.f }},
		{{ -drawSizeRaito.x, +drawSizeRaito.y, 0.f }, { 0.f, 0.f }},
		{{ +drawSizeRaito.x, -drawSizeRaito.y, 0.f }, { 1.f, 1.f }},
		{{ +drawSizeRaito.x, +drawSizeRaito.y, 0.f }, { 1.f, 0.f }},
	};

	// 頂点バッファへデータ転送
	VertexPosUv* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result))
	{
		memcpy(vertMap, vertices, sizeof(vertices));
		vertBuff->Unmap(0, nullptr);
	}

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(VertexPosUv) * vertNum;
	vbView.StrideInBytes = sizeof(VertexPosUv);

	// 定数バッファの生成
	result = DX12Base::getInstance()->getDev()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff)
	);
	assert(SUCCEEDED(result));

	transferConstBuff((float)timer->getNowTime());
}

void PostEffect::createGraphicsPipelineState(const wchar_t* psPath)
{
	ComPtr<ID3DBlob> vsBlob = nullptr; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob = nullptr; // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob = nullptr; // エラーオブジェクト

	constexpr UINT compileFlag =
#ifdef _DEBUG
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		0;
#endif // _DEBUG

	constexpr const char hlslData[] = "\n\
#include \"PostEffect.hlsli\"\n\
VSOutput main(float4 pos : POSITION, float2 uv : TEXCOORD)\n\
	{\n\
		VSOutput output; // ピクセルシェーダーに渡す値\n\
		output.svpos = pos;\n\
		output.uv = uv;\n\
		return output;\n\
	}";

	HRESULT result = D3DCompile(
		hlslData,
		_countof(hlslData),
		"Resources/Shaders/",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		compileFlag,
		0,
		&vsBlob,
		&errorBlob
	);

	if (FAILED(result))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		assert(0);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		psPath,   // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
		compileFlag,
		0,
		&psBlob, &errorBlob);

	if (FAILED(result))
	{
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		assert(0);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};

	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;              // 背面カリングをしない

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = gpipeline.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // 標準設定
	blenddesc.BlendEnable = true;                   // ブレンドを有効にする
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;    // 加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;      // ソースの値を100% 使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;    // デストの値を   0% 使う

	//--半透明合成
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;				//加算
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			//ソースのアルファ値
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	//デストの値を100%使う

	// デプスステンシルステートの設定
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DepthStencilState.DepthEnable = false;    // 深度テストをしない
	//gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;       // 常に上書きルール
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT; // 深度値フォーマット

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1; // 描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	// デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV0{};
	descRangeSRV0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	CD3DX12_DESCRIPTOR_RANGE descRangeSRV1{};
	descRangeSRV1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // t1 レジスタ

	// ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[3]{};
	rootparams[0].InitAsConstantBufferView(0); // 定数バッファビューとして初期化(b0レジスタ)
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV0);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV1);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ルートシグネチャの生成
	// ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定でのシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
												   D3D_ROOT_SIGNATURE_VERSION_1_0,
												   &rootSigBlob,
												   &errorBlob);
	assert(SUCCEEDED(result));

	// ルートシグネチャの生成
	result = DX12Base::getInstance()->getDev()->CreateRootSignature(0,
																	rootSigBlob->GetBufferPointer(),
																	rootSigBlob->GetBufferSize(),
																	IID_PPV_ARGS(&pipelineSet.back().rootsignature));
	assert(SUCCEEDED(result));

	// パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = pipelineSet.back().rootsignature.Get();

	result = DX12Base::getInstance()->getDev()->CreateGraphicsPipelineState(&gpipeline,
																			IID_PPV_ARGS(&pipelineSet.back().pipelinestate));
	assert(SUCCEEDED(result));
}

size_t PostEffect::addPipeLine(const wchar_t* psPath)
{
	pipelineSet.emplace_back();
	createGraphicsPipelineState(psPath);

	return pipelineSet.size() - 1u;
}

void PostEffect::init()
{
	createGraphicsPipelineState();

	initBuffer();

	// テクスチャリソース設定
	CD3DX12_RESOURCE_DESC texresDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
									 WinAPI::window_width, WinAPI::window_height,
									 1, 0, 1, 0,
									 D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	// テクスチャバッファ設定
	HRESULT result;
	for (UINT i = 0; i < renderTargetNum; i++)
	{
		result =
			dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
																  D3D12_MEMORY_POOL_L0),
										 D3D12_HEAP_FLAG_NONE,
										 &texresDesc,
										 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
										 &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM,
															  clearColor),
										 IID_PPV_ARGS(&texbuff[i]));

		assert(SUCCEEDED(result));

		{
			// 画素数
			constexpr UINT pixelCount = WinAPI::window_width * WinAPI::window_height;
			// 一行分のデータサイズ
			constexpr UINT rowPitch = sizeof(UINT) * WinAPI::window_width;
			// 画像全体のデータサイズ
			constexpr UINT depthPitch = rowPitch * WinAPI::window_height;
			// 画像イメージ
			UINT* img = new UINT[pixelCount]{};
			// 0xrrggbbaaの色にする
			for (UINT j = 0; j < pixelCount; j++)
			{
				img[j] = 0xffffffff;
			}
			// テクスチャバッファにデータ転送
			result = texbuff[i]->WriteToSubresource(0, nullptr, img, rowPitch, depthPitch);
			delete[] img;

			assert(SUCCEEDED(result));
		}
	}

	// SRV用デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC srvDescHeapDesc{};
	srvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescHeapDesc.NumDescriptors = 2;
	// SRV用デスクリプタヒープを生成
	result = dev->CreateDescriptorHeap(&srvDescHeapDesc,
									   IID_PPV_ARGS(&descHeapSRV));
	assert(SUCCEEDED(result));

	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// テクスチャバッファと同じ
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// デスクリプタヒープにSRV作成
	for (UINT i = 0; i < renderTargetNum; i++)
	{
		dev->CreateShaderResourceView(texbuff[i].Get(),
									  &srvDesc,
									  CD3DX12_CPU_DESCRIPTOR_HANDLE(
										  descHeapSRV->GetCPUDescriptorHandleForHeapStart(),
										  i,
										  dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
									  )
		);
	}

	// RTV用デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc{};
	rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHeapDesc.NumDescriptors = renderTargetNum;
	// RTV用デスクリプタヒープを生成
	result = dev->CreateDescriptorHeap(&rtvDescHeapDesc, IID_PPV_ARGS(&descHeapRTV));
	assert(SUCCEEDED(result));
	for (UINT i = 0; i < renderTargetNum; i++)
	{
		// デスクリプタヒープにRTVを作成
		dev->CreateRenderTargetView(texbuff[i].Get(),
									nullptr,
									CD3DX12_CPU_DESCRIPTOR_HANDLE(
										descHeapRTV->GetCPUDescriptorHandleForHeapStart(),
										i,
										dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
		);
	}
	// 深度バッファのリソース設定
	CD3DX12_RESOURCE_DESC depthResDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT,
			WinAPI::window_width,
			WinAPI::window_height,
			1, 0,
			1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);
	// 深度バッファの生成
	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.f, 0),
		IID_PPV_ARGS(&depthBuff)
	);
	assert(SUCCEEDED(result));

	// DSV用のデスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descHeapDesc.NumDescriptors = 1;
	// DSV用のデスクリプタヒープを作成
	result = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeapDSV));
	assert(SUCCEEDED(result));

	// デスクリプタヒープにDSVを作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;	// 深度値フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(depthBuff.Get(),
								&dsvDesc,
								descHeapDSV->GetCPUDescriptorHandleForHeapStart());
}

void PostEffect::draw(DX12Base* dxBase)
{
	transferConstBuff((float)timer->getNowTime());

#pragma region 描画設定

	// パイプラインステートの設定
	cmdList->SetPipelineState(pipelineSet[nowPPSet].pipelinestate.Get());
	// ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(pipelineSet[nowPPSet].rootsignature.Get());
	// プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// テクスチャ用デスクリプタヒープの設定
	ID3D12DescriptorHeap* ppHeaps[] = { descHeapSRV.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

#pragma endregion 描画設定

#pragma region 描画
	// 頂点バッファをセット
	cmdList->IASetVertexBuffers(0, 1, &vbView);

	// 定数バッファをセット
	cmdList->SetGraphicsRootConstantBufferView(0, constBuff->GetGPUVirtualAddress());

	// シェーダリソースビューをセット
	cmdList->SetGraphicsRootDescriptorTable(1,
											CD3DX12_GPU_DESCRIPTOR_HANDLE(
												descHeapSRV->GetGPUDescriptorHandleForHeapStart(),
												0,
												dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
											)
	);
	cmdList->SetGraphicsRootDescriptorTable(2,
											CD3DX12_GPU_DESCRIPTOR_HANDLE(
												descHeapSRV->GetGPUDescriptorHandleForHeapStart(),
												1,
												dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
											)
	);

	// ポリゴンの描画（4頂点で四角形）
	cmdList->DrawInstanced(4, 1, 0, 0);

#pragma endregion 描画
}

void PostEffect::startDrawScene(DX12Base* dxBase)
{
	// リソースバリアを変更(シェーダーリソース -> 描画可能)
	for (UINT i = 0; i < renderTargetNum; i++)
	{
		cmdList->ResourceBarrier(1,
								 &CD3DX12_RESOURCE_BARRIER::Transition(texbuff[i].Get(),
																	   D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
																	   D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
	// レンダーターゲットビュー用デスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHs[renderTargetNum]{};
	for (UINT i = 0; i < renderTargetNum; i++)
	{
		rtvHs[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			descHeapRTV->GetCPUDescriptorHandleForHeapStart(), i,
			DX12Base::getInstance()->getDev()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	}

	// 深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH =
		descHeapDSV->GetCPUDescriptorHandleForHeapStart();

	// レンダーターゲットをセット
	cmdList->OMSetRenderTargets(renderTargetNum, rtvHs, false, &dsvH);

	CD3DX12_VIEWPORT viewPorts[renderTargetNum]{};
	CD3DX12_RECT scissorRects[renderTargetNum]{};

	for (UINT i = 0; i < renderTargetNum; i++)
	{
		viewPorts[i] = CD3DX12_VIEWPORT(0.f, 0.f, WinAPI::window_width, WinAPI::window_height);
		scissorRects[i] = CD3DX12_RECT(0, 0, WinAPI::window_width, WinAPI::window_height);
	}

	// ビューポートの設定
	cmdList->RSSetViewports(renderTargetNum, viewPorts);
	// シザリング矩形の設定
	cmdList->RSSetScissorRects(renderTargetNum, scissorRects);
	// 全画面クリア
	for (UINT i = 0; i < renderTargetNum; i++)
	{
		cmdList->ClearRenderTargetView(rtvHs[i], clearColor, 0, nullptr);
	}

	// 深度バッファのクリア
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
}

void PostEffect::endDrawScene(DX12Base* dxBase)
{
	for (UINT i = 0; i < renderTargetNum; i++)
	{
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texbuff[i].Get(),
																		  D3D12_RESOURCE_STATE_RENDER_TARGET,
																		  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}