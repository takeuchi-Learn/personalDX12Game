#include "Object3d.h"

#include <d3dx12.h>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "PostEffect.h"

using namespace DirectX;
using namespace Microsoft::WRL;

ID3D12Device* Object3d::dev = nullptr;
Object3d::PipelineSet Object3d::ppSetDef{};
Camera* Object3d::camera = nullptr;

void Object3d::createTransferBufferB0(ID3D12Device* dev, ComPtr<ID3D12Resource>& constBuffB0) {
	// 定数バッファの生成
	HRESULT result = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),   // アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB0) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB0)
	);
}

XMMATRIX Object3d::getMatWorld() const { return matWorld; }

//void Object3d::setTexture(ID3D12Device* dev, const UINT newTexNum) {
//	texNum = newTexNum;
//	//model->setTexture(dev, newTexNum, constantBufferNum);
//}

Object3d::Object3d(ID3D12Device* dev, Camera* camera) {

	matWorld = {};

	// 定数バッファの生成
	createTransferBufferB0(dev, constBuffB0);
}
Object3d::Object3d(ID3D12Device* dev, Camera* camera, ObjModel* model, const UINT texNum) : texNum(texNum) {
	matWorld = {};

	// 定数バッファの生成
	createTransferBufferB0(dev, constBuffB0);

	this->model = model;
	this->camera = camera;
}

void Object3d::update(ID3D12Device* dev) {
	XMMATRIX matScale, matRot, matTrans;

	// スケール、回転、平行移動行列の計算
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	// ワールド行列の合成
	matWorld = XMMatrixIdentity(); // 変形をリセット
	matWorld *= matScale; // ワールド行列にスケーリングを反映
	matWorld *= matRot; // ワールド行列に回転を反映
	if (isBillboard) {
		const XMMATRIX& matBillBoard = camera->getBillboardMatrix();
		matWorld *= matBillBoard;
	} else if (isBillBoardY) {
		const XMMATRIX& matBillBoardY = camera->getBillboardMatrixY();
		matWorld *= matBillBoardY;
	}
	matWorld *= matTrans; // ワールド行列に平行移動を反映

	// 親オブジェクトがあれば
	if (parent != nullptr) {
		// 親オブジェクトのワールド行列を掛ける
		matWorld *= parent->matWorld;
	}

	// 定数バッファへデータ転送
	ConstBufferDataB0* constMapB0 = nullptr;
	if (SUCCEEDED(constBuffB0->Map(0, nullptr, (void**)&constMapB0))) {
		//constMap->color = color; // RGBA
		constMapB0->viewProj = camera->getViewProjectionMatrix();
		constMapB0->world = matWorld;
		constMapB0->cameraPos = camera->getEye();
		constBuffB0->Unmap(0, nullptr);
	}
	/*ConstBufferDataB1* constMapB1 = nullptr;
	HRESULT result = constBuffB1->Map(0, nullptr, (void**)&constMapB1);
	constMapB1->ambient = material.ambient;
	constMapB1->diffuse = material.diffuse;
	constMapB1->specular = material.specular;
	constMapB1->alpha = material.alpha;
	constBuffB1->Unmap(0, nullptr);*/

	//model->update(dev);
}

void Object3d::draw(DX12Base* dxBase, Light* light) {
	// 定数バッファビューをセット
	dxBase->getCmdList()->SetGraphicsRootConstantBufferView(0, constBuffB0->GetGPUVirtualAddress());

	light->draw(dxBase, 3);

	model->draw(dxBase->getCmdList());
}

void Object3d::drawWithUpdate(DX12Base* dxBase, Light* light) {
	update(dxBase->getDev());
	draw(dxBase, light);
}

Object3d::~Object3d() {}



void Object3d::startDraw(ID3D12GraphicsCommandList* cmdList, Object3d::PipelineSet& ppSet, D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology) {
	cmdList->SetPipelineState(ppSet.pipelinestate.Get());
	cmdList->SetGraphicsRootSignature(ppSet.rootsignature.Get());
	//プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(PrimitiveTopology);
}

void Object3d::staticInit(ID3D12Device* device) {
	// 再初期化チェック
	assert(!Object3d::dev);

	// nullptrチェック
	assert(device);

	Object3d::dev = device;

	ppSetDef = createGraphicsPipeline(device);

	ObjModel::staticInit(device);
}

Object3d::PipelineSet Object3d::createGraphicsPipeline(ID3D12Device* dev,
													   BLEND_MODE blendMode,
													   const wchar_t* vsShaderPath, const wchar_t* psShaderPath) {
	HRESULT result = S_FALSE;

	ComPtr<ID3DBlob> vsBlob;	// 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;		// ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob;	// エラーオブジェクト

	//頂点シェーダーの読み込みとコンパイル
	result = D3DCompileFromFile(
		vsShaderPath,  // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);
	//エラー表示
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		psShaderPath,   // シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0", // エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);

	//エラー表示
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
					errorBlob->GetBufferSize(),
					errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// 頂点レイアウト
	// --- 頂点シェーダーの引数と対応
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};	// 1行で書いたほうが見やすい

	// グラフィックスパイプライン設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	//頂点シェーダー、ピクセルシェーダ
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
	//標準的な設定(背面カリング、塗りつぶし、深度クリッピング有効)
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定

	//ラスタライザステート
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	//デプスステンシルステートの設定
	//標準的な設定(深度テストを行う、書き込み許可、深度が小さければ合格)
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	//レンダ―ターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; //標準設定
	blenddesc.BlendEnable = true;	//ブレンドを有効にする

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;	//加算
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;		//ソースの値を100%使う
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;	//デストの値を0%使う

	switch (blendMode) {
	case Object3d::BLEND_MODE::ADD:
		//---加算
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;				// 加算
		blenddesc.SrcBlend = D3D12_BLEND_ONE;				// ソースの値を100%使う
		blenddesc.DestBlend = D3D12_BLEND_ONE;				// デストの値を100%使う
		break;
	case Object3d::BLEND_MODE::SUB:
		//---減算
		blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;	// デストからソースを減算
		blenddesc.SrcBlend = D3D12_BLEND_ONE;				// ソースの値を100%使う
		blenddesc.DestBlend = D3D12_BLEND_ONE;				// デストの値を100%使う
		break;
	case Object3d::BLEND_MODE::REVERSE:
		//---反転
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;				// 加算
		blenddesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;	// 1.0 - デストカラーの値
		blenddesc.DestBlend = D3D12_BLEND_ZERO;
		break;
	case Object3d::BLEND_MODE::ALPHA:
		//--半透明合成
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;				// 加算
		blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			// ソースのアルファ値
		blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	// デストの値を100%使う
		break;
	default:
		//--半透明合成
		blenddesc.BlendOp = D3D12_BLEND_OP_ADD;				// 加算
		blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;			// ソースのアルファ値
		blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;	// デストの値を100%使う
		break;
	}

	// ブレンドステートの設定
	for (UINT i = 0, maxSize = _countof(gpipeline.BlendState.RenderTarget);
		 i < PostEffect::renderTargetNum && i < maxSize;
		 i++) {
		gpipeline.BlendState.RenderTarget[i] = blenddesc;
	}

	// 深度バッファのフォーマット
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = PostEffect::renderTargetNum; // 描画対象の数

	for (UINT i = 0, maxSize = _countof(gpipeline.BlendState.RenderTarget);
		 i < PostEffect::renderTargetNum && i < maxSize;
		 i++) {
		gpipeline.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0～255指定のRGBA
	}

	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	//デスクリプタテーブルの設定
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);//t0レジスタ

	//ルートパラメータの設定
	CD3DX12_ROOT_PARAMETER rootparams[4]{};
	rootparams[0].InitAsConstantBufferView(0); //定数バッファビューとして初期化(b0レジスタ)
	rootparams[1].InitAsConstantBufferView(1); //定数バッファビューとして初期化(b0レジスタ)
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV); //テクスチャ用
	rootparams[3].InitAsConstantBufferView(2);

	//テクスチャサンプラーの設定
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	Object3d::PipelineSet pipelineSet{};

	//ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> rootSigBlob;
	//バージョン自動判定でのシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	//ルートシグネチャの生成
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pipelineSet.rootsignature));
	if (FAILED(result)) {
		assert(0);
	}
	// パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = pipelineSet.rootsignature.Get();

	//パイプラインステートの生成
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelineSet.pipelinestate));

	if (FAILED(result)) {
		assert(0);
	}

	return pipelineSet;
}