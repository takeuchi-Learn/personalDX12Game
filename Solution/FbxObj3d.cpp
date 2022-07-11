#include "FbxObj3d.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "FbxLoader.h"

#include "PostEffect.h"

using namespace Microsoft::WRL;
using namespace DirectX;

ID3D12Device *FbxObj3d::dev = nullptr;
Camera *FbxObj3d::camera = nullptr;

ComPtr<ID3D12RootSignature> FbxObj3d::rootsignature;
ComPtr<ID3D12PipelineState> FbxObj3d::pipelinestate;

void FbxObj3d::createGraphicsPipeline(const wchar_t *vsPath, const wchar_t *psPath) {
	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;    // ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

	assert(dev);

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		vsPath,    // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0",    // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char *)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		psPath,    // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0",    // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char *)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy座標(1行で書いたほうが見やすい)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // 法線ベクトル(1行で書いたほうが見やすい)
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // uv座標(1行で書いたほうが見やすい)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{	// 影響を受けるボーン番号(4つ)
			"BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{	// 影響を受けるボーン番号(4つ)
			"BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	// グラフィックスパイプラインの流れを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	// サンプルマスク
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	// ラスタライザステート
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// デプスステンシルステート
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;    // RBGA全てのチャンネルを描画
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	// ブレンドステートの設定
	for (UINT i = 0, maxSize = _countof(gpipeline.BlendState.RenderTarget);
		 i < PostEffect::renderTargetNum && i < maxSize;
		 i++) {
		gpipeline.BlendState.RenderTarget[i] = blenddesc;
	}

	// 深度バッファのフォーマット
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// 頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定（三角形）
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = PostEffect::renderTargetNum;    // 描画対象の数
	for (UINT i = 0, maxSize = _countof(gpipeline.BlendState.RenderTarget);
		 i < PostEffect::renderTargetNum && i < maxSize;
		 i++) {
		gpipeline.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA
	}

	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	// デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	// ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootparams[5]{};
	// CBV（座標変換行列用）
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	// SRV（テクスチャ）
	rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);
	// CBV(スキニング用)
	rootparams[2].InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[3].InitAsConstantBufferView(1);
	rootparams[4].InitAsConstantBufferView(2);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	// ルートシグネチャの生成
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootsignature.ReleaseAndGetAddressOf()));
	if (FAILED(result)) { assert(0); }

	gpipeline.pRootSignature = rootsignature.Get();

	// グラフィックスパイプラインの生成
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(pipelinestate.ReleaseAndGetAddressOf()));
	if (FAILED(result)) { assert(0); }
}



FbxObj3d::FbxObj3d(bool animLoop) : animLoop(animLoop) {
	init();
}
FbxObj3d::FbxObj3d(FbxModel *model, bool animLoop) : animLoop(animLoop) {
	init();
	setModel(model);
}

void FbxObj3d::init() {
	HRESULT result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataTransform) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffTransform)
	);

	result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataSkin) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffSkin)
	);

	// ungdone 1フレームの時間は60FPSを想定して固定
	frameTime.SetTime(0, 0, 0, 1, 0, FbxTime::EMode::eFrames60);

	// スキン無しへの対応のため、定数バッファへデータ転送
	ConstBufferDataSkin *constMapSkin = nullptr;
	result = constBuffSkin->Map(0, nullptr, (void **)&constMapSkin);
	for (int i = 0; i < MAX_BONES; i++) {
		constMapSkin->bones[i] = XMMatrixIdentity();
	}
	constBuffSkin->Unmap(0, nullptr);
}

void FbxObj3d::update() {
	// アニメーション再生中ならフレーム数を進める
	if (isPlay) {
		currentTime += frameTime;
		// ループする場合、終了したら初めから
		if (currentTime > endTime) {
			if (animLoop) {
				currentTime = startTime;
			} else {
				isPlay = false;
			}
		}
	}

	XMMATRIX matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	XMMATRIX matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	matWorld = XMMatrixIdentity();
	matWorld *= matScale;
	matWorld *= matRot;
	matWorld *= matTrans;

	const XMMATRIX &matViewProj = camera->getViewProjectionMatrix();
	// モデルのメッシュのトランスフォーム
	const XMMATRIX &modelTransform = model->GetModelTransform();
	// カメラ座標
	const XMFLOAT3 &cameraPos = camera->getEye();

	// 定数バッファへデータを転送
	ConstBufferDataTransform *constMap = nullptr;
	HRESULT result = constBuffTransform->Map(0, nullptr, (void **)&constMap);
	if (SUCCEEDED(result)) {
		constMap->viewproj = matViewProj;
		constMap->world = modelTransform * matWorld;
		constMap->cameraPos = cameraPos;
		constBuffTransform->Unmap(0, nullptr);
	}

	// ボーン配列
	std::vector<FbxModel::Bone> &bones = model->getBones();

	// 定数バッファへデータ転送
	ConstBufferDataSkin *constMapSkin = nullptr;
	result = constBuffSkin->Map(0, nullptr, (void **)&constMapSkin);
	for (UINT i = 0, loopLen = UINT(bones.size()); i < loopLen; i++) {
		// 今の姿勢
		XMMATRIX matCurrentPose{};
		// 今の姿勢を取得
		FbxAMatrix fbxCurrentPose =
			bones[i].fbxCluster->GetLink()->EvaluateGlobalTransform(currentTime);
		// XMMATRIXに変換
		FbxLoader::convertMatrixFromFbx(&matCurrentPose, fbxCurrentPose);
		// 合成してスキニング行列に
		// メッシュノードのグローバルトランスフォーム *
		// ボーンの初期姿勢の逆行列 *
		// ボーンの今の姿勢 *
		// メッシュノードのグローバルトランスフォームの逆行列
		constMapSkin->bones[i] = model->GetModelTransform() *
			bones[i].invInitialPose
			* matCurrentPose
			* XMMatrixInverse(nullptr, model->GetModelTransform());
	}
	constBuffSkin->Unmap(0, nullptr);
}

void FbxObj3d::draw(ID3D12GraphicsCommandList *cmdList, Light *light) {
	//　モデルがないなら描画しない
	if (model == nullptr) return;

	assert(light != nullptr);

	// パイプラインステートの設定
	cmdList->SetPipelineState(pipelinestate.Get());
	// ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(rootsignature.Get());
	// プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 定数バッファビューをセット
	cmdList->SetGraphicsRootConstantBufferView(0, constBuffTransform->GetGPUVirtualAddress());
	// --- 第一引数はcreateGraphicsPipeliine内rootparamsの該当する要素番号
	cmdList->SetGraphicsRootConstantBufferView(2, constBuffSkin->GetGPUVirtualAddress());

	light->draw(DX12Base::getInstance(), 4);

	cmdList->SetGraphicsRootConstantBufferView(3, model->getConstBuffB1()->GetGPUVirtualAddress());

	// モデルを描画
	model->draw(cmdList);
}

void FbxObj3d::drawWithUpdate(ID3D12GraphicsCommandList *cmdList, Light *light) {
	update();
	draw(cmdList, light);
}

void FbxObj3d::playAnimation() {
	FbxScene *fbxScene = model->getFbxScene();
	// 0番のアニメーションを取得
	FbxAnimStack *animStack = fbxScene->GetSrcObject<FbxAnimStack>(0);
	if (animStack == nullptr) return;
	// アニメーションの名前を取得
	const char *animStackName = animStack->GetName();
	// アニメーションの時間取得
	FbxTakeInfo *takeInfo = fbxScene->GetTakeInfo(animStackName);

	// 開始時間を取得
	startTime = takeInfo->mLocalTimeSpan.GetStart();
	// 終了時間を取得
	endTime = takeInfo->mLocalTimeSpan.GetStop();
	// 現在の時間を開始時間にする
	currentTime = startTime;
	// 再生状態にする
	isPlay = true;
}

void FbxObj3d::stopAnimation(bool resetPoseFlag) {
	isPlay = false;
	if(resetPoseFlag) currentTime = startTime;
}
