#include "FbxLoader.h"
#include <cassert>

using namespace DirectX;

const std::string FbxLoader::baseDir = "Resources/";
const std::string FbxLoader::defaultTextureFileName = "white.png";

FbxLoader* FbxLoader::GetInstance() {
	static FbxLoader instance;
	return &instance;
}

void FbxLoader::convertMatrixFromFbx(DirectX::XMMATRIX* dst,
									 const FbxAMatrix& src) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			dst->r[y].m128_f32[x] = (float)src.Get(y, x);
		}
	}
}

void FbxLoader::init(ID3D12Device* dev) {
	assert(fbxManager == nullptr);
	this->dev = dev;
	// マネージャーの生成
	fbxManager = FbxManager::Create();
	// マネージャーの入出力設定
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ios);
	// インポーターの設定
	fbxImporter = FbxImporter::Create(fbxManager, "");
}

void FbxLoader::fin() {
	// インスタンスの破棄
	fbxImporter->Destroy();
	fbxManager->Destroy();
}

FbxModel* FbxLoader::loadModelFromFile(const std::string& modelName) {
	const std::string dirPath = baseDir + modelName + "/";
	// 拡張子を付加
	const std::string fileName = modelName + ".fbx";
	// 繋げてフルパスを得る
	const std::string fullPath = dirPath + fileName;

	// ファイル名を指定してファイル読み込み
	if (!fbxImporter->Initialize(fullPath.c_str(),
								 -1,
								 fbxManager->GetIOSettings())) {
		assert(0);
	}

	// シーン生成
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "fbxScene");
	//ロードした情報をインポート
	fbxImporter->Import(fbxScene);

	// モデル生成
	FbxModel* model = new FbxModel();
	model->name = modelName;

	// ノードの数を取得
	int nodeCount = fbxScene->GetNodeCount();
	// 予め必要分のメモリを確保し、アドレスのずれを予防
	model->nodes.reserve(nodeCount);

	// ルートノードから順に解析しモデルに流し込む
	parseNodeRecursive(model, fbxScene->GetRootNode());

	model->fbxScene = fbxScene;

	model->createBuffers(dev);

	return model;
}

void FbxLoader::parseNodeRecursive(FbxModel* model,
								   FbxNode* fbxNode,
								   FbxModel::Node* parent) {
	// モデルにノードを追加
	model->nodes.emplace_back();
	FbxModel::Node& node = model->nodes.back();
	// ノード名を取得
	node.name = fbxNode->GetName();
	// ノードのローカル移動情報
	FbxDouble3 rotation = fbxNode->LclRotation.Get();
	FbxDouble3 scaling = fbxNode->LclScaling.Get();
	FbxDouble3 translation = fbxNode->LclTranslation.Get();
	// 形式変換して代入
	node.rotation = { (float)rotation[0], (float)rotation[1], (float)rotation[2], 0.f };
	node.scaling = { (float)scaling[0], (float)scaling[1], (float)scaling[2], 0.f };
	node.translation = { (float)translation[0], (float)translation[1], (float)translation[2], 1.f };

	// 回転角を弧度法に変換
	node.rotation.m128_f32[0] = XMConvertToRadians(node.rotation.m128_f32[0]);
	node.rotation.m128_f32[1] = XMConvertToRadians(node.rotation.m128_f32[1]);
	node.rotation.m128_f32[2] = XMConvertToRadians(node.rotation.m128_f32[2]);

	// スケール、回転、平行移動行列の計算
	XMMATRIX matScaling, matRotation, matTranslation;
	matScaling = XMMatrixScalingFromVector(node.scaling);
	matRotation = XMMatrixRotationRollPitchYawFromVector(node.rotation);
	matTranslation = XMMatrixTranslationFromVector(node.translation);

	// ローカル変形行列の計算
	node.transform = XMMatrixIdentity();
	node.transform *= matScaling;
	node.transform *= matRotation;
	node.transform *= matTranslation;

	// グローバル変形行列の計算
	node.globalTransform = node.transform;
	if (parent) {
		node.parent = parent;
		node.globalTransform *= parent->globalTransform;
	}

	// fbxノードのメッシュ構造を解析
	FbxNodeAttribute* fbxNodeAttribure = fbxNode->GetNodeAttribute();
	if (fbxNodeAttribure) {
		if (fbxNodeAttribure->GetAttributeType() == FbxNodeAttribute::eMesh) {
			model->meshNode = &node;
			parseMesh(model, fbxNode);
		}
	}

	// 子ノードに対して再帰呼び出し
	for (int i = 0; i < fbxNode->GetChildCount(); i++) {
		parseNodeRecursive(model, fbxNode->GetChild(i), &node);
	}
}

void FbxLoader::parseMesh(FbxModel* model, FbxNode* fbxNode) {
	// ノードのメッシュ取得
	FbxMesh* fbxMesh = fbxNode->GetMesh();

	// 頂点情報読み取り
	parseMeshVertices(model, fbxMesh);

	// 面を構成するデータの読み取り
	parseMeshFaces(model, fbxMesh);

	// マテリアル読み込み
	parseMaterial(model, fbxNode);

	// スキニング情報読み込み
	parseSkin(model, fbxMesh);
}

void FbxLoader::parseMeshVertices(FbxModel* model, FbxMesh* fbxMesh) {
	auto& vertices = model->vertices;

	// 頂点データの数
	const int controlPointCount = fbxMesh->GetControlPointsCount();

	//必要数だけ頂点データ配列を確保
	FbxModel::VertexPosNormalUvSkin vert{};
	model->vertices.resize(controlPointCount, vert);

	// FBXメッシュの頂点座標配列を取得
	FbxVector4* pCoord = fbxMesh->GetControlPoints();

	// メッシュの全頂点座標をモデル内配列にコピー
	for (int i = 0; i < controlPointCount; i++) {
		FbxModel::VertexPosNormalUvSkin& vertex = vertices[i];

		// 座標をコピー
		vertex.pos.x = (float)pCoord[i][0];
		vertex.pos.y = (float)pCoord[i][1];
		vertex.pos.z = (float)pCoord[i][2];
	}
}

void FbxLoader::parseMeshFaces(FbxModel* model, FbxMesh* fbxMesh) {
	auto& vertices = model->vertices;
	auto& indices = model->indices;

	// 複数メッシュのモデルには非対応
	assert(indices.size() == 0);

	// 面の数
	const int polygonCount = fbxMesh->GetPolygonCount();

	// uvデータの数
	const int textureUVCount = fbxMesh->GetTextureUVCount();

	// uv名リスト
	FbxStringList uvNames;
	fbxMesh->GetUVSetNames(uvNames);

	// 面ごとの情報読み取り
	for (int i = 0; i < polygonCount; i++) {
		// 面を構成する頂点数を取得(3なら三角形ポリゴン)
		const int polygonSize = fbxMesh->GetPolygonSize(i);
		assert(polygonSize <= 4);

		// 1頂点ずつ
		for (int j = 0; j < polygonSize; j++) {
			// fbx頂点配列のインデックス
			int index = fbxMesh->GetPolygonVertex(i, j);
			assert(index >= 0);

			// 頂点法線読み込み
			FbxModel::VertexPosNormalUvSkin& vertex = vertices[index];
			FbxVector4 normal;
			if (fbxMesh->GetPolygonVertexNormal(i, j, normal)) {
				vertex.normal.x = (float)normal[0];
				vertex.normal.y = (float)normal[1];
				vertex.normal.z = (float)normal[2];
			}

			// テクスチャUV読み込み
			if (textureUVCount > 0) {
				FbxVector2 uvs;
				bool lUnmappedUV;
				// 0番決め打ちで読み込み
				if (fbxMesh->GetPolygonVertexUV(i, j, uvNames[0], uvs, lUnmappedUV)) {
					vertex.uv.x = (float)uvs[0];
					vertex.uv.y = (float)uvs[1];
				}
			}

			// インデックス配列に頂点インデックスを追加
			// 3頂点目までなら
			if (j < 3) {
				indices.push_back(index);
			} else {
				// 3点追加し、四角形の0,1,2,3の内2,3,0で三角形を構築する
				int index2 = indices[indices.size() - 1];
				int index3 = index;
				int index0 = indices[indices.size() - 3];
				indices.push_back(index2);
				indices.push_back(index3);
				indices.push_back(index0);
			}
		}
	}
}

void FbxLoader::parseMaterial(FbxModel* model, FbxNode* fbxNode) {
	const int materialCount = fbxNode->GetMaterialCount();
	if (materialCount > 0) {
		// 先頭のマテリアルを取得
		FbxSurfaceMaterial* material = fbxNode->GetMaterial(0);
		// テクスチャを読み込んだかを示すフラグ
		bool textureLoaded = false;
		if (material) {
			// FbxSurfaceLambertクラスかどうかを調べる
			if (material->GetClassId().Is(FbxSurfaceLambert::ClassId)) {
				FbxSurfaceLambert* lambert = static_cast<FbxSurfaceLambert*>(material);

				// 環境光係数
				FbxPropertyT<FbxDouble3> ambient = lambert->Ambient;
				model->ambient.x = (float)ambient.Get()[0];
				model->ambient.y = (float)ambient.Get()[1];
				model->ambient.z = (float)ambient.Get()[2];

				// 拡散反射光係数
				FbxPropertyT<FbxDouble3> diffuse = lambert->Diffuse;
				model->diffuse.x = (float)diffuse.Get()[0];
				model->diffuse.y = (float)diffuse.Get()[1];
				model->diffuse.z = (float)diffuse.Get()[2];

				FbxSurfacePhong* phong = (FbxSurfacePhong*)material;

				FbxPropertyT<FbxDouble3> specular = phong->Specular;
				model->specular.x = (float)specular.Get()[0];
				model->specular.y = (float)specular.Get()[1];
				model->specular.z = (float)specular.Get()[2];
			}
			// ディフューズテクスチャを取り出す
			const FbxProperty diffuseProperty = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if (diffuseProperty.IsValid()) {
				const FbxFileTexture* texture = diffuseProperty.GetSrcObject<FbxFileTexture>();
				if (texture) {
					const char* filePath = texture->GetFileName();
					// ファイルパスからファイル名を抽出
					std::string path_str(filePath);
					std::string name = ExtractFileName(path_str);
					// テクスチャ読み込み
					loadTexture(model, baseDir + model->name + "/" + name);
					textureLoaded = true;
				}
			}
		}
		// テクスチャがない場合
		if (!textureLoaded) {
			loadTexture(model, baseDir + defaultTextureFileName);
		}
	} else {
		// マテリアルがない場合はデフォルトのテクスチャを読み込む
		// アンビエントなどの値はヘッダーで値を入れてある
		loadTexture(model, baseDir + defaultTextureFileName);
	}
}

std::string FbxLoader::ExtractFileName(const std::string& path) {
	size_t pos1;
	// 区切り文字 '\\'が出る一番最後の部分を検索
	pos1 = path.rfind('\\');
	if (pos1 != std::string::npos) {
		return path.substr(pos1 + 1, path.size() - pos1 - 1);
	}
	// 区切り文字 '/'が出る一番最後の部分を検索
	pos1 = path.rfind('/');
	if (pos1 != std::string::npos) {
		return path.substr(pos1 + 1, path.size() - pos1 - 1);
	}

	return path;
}

void FbxLoader::parseSkin(FbxModel* model,
						  FbxMesh* fbxMesh) {
	FbxSkin* fbxSkin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
	// スキニング情報がなければ終了
	if (fbxSkin == nullptr) {
		for (auto& i : model->vertices) {
			i.boneIndex[0] = 0;
			i.boneWeight[0] = 1.f;
		}
		return;
	}
	// ボーン配列の参照
	std::vector<FbxModel::Bone>& bones = model->bones;

	// ボーンの数
	int clusterCount = fbxSkin->GetClusterCount();
	bones.reserve(clusterCount);

	for (int i = 0; i < clusterCount; i++) {
		FbxCluster* fbxCluster = fbxSkin->GetCluster(i);
		// ボーン自体のノードの名前を取得
		const char* boneName = fbxCluster->GetLink()->GetName();
		//ボーンを追加し、関連したボーンの参照を得る
		bones.emplace_back(FbxModel::Bone(boneName));
		FbxModel::Bone& bone = bones.back();
		// 自作ボーンとBXのボーンを紐づける
		bone.fbxCluster = fbxCluster;
		// FBXから初期姿勢行列を取得
		FbxAMatrix fbxMat;
		fbxCluster->GetTransformLinkMatrix(fbxMat);
		// XMMATRIXに変換
		XMMATRIX initialPose;
		convertMatrixFromFbx(&initialPose, fbxMat);
		// 初期姿勢の逆行列を取得
		bone.invInitialPose = XMMatrixInverse(nullptr, initialPose);
	}
	// ボーン番号とスキンウェイトのペア
	struct WeightSet {
		UINT index;
		float weight;
	};
	// ジャグ配列
	// list : 頂点が影響を受けるボーンのリスト
	// vector : listを全頂点分
	std::vector<std::list<WeightSet>> weightLists(model->vertices.size());

	for (int i = 0; i < clusterCount; i++) {
		FbxCluster* fbxCluster = fbxSkin->GetCluster(i);
		// 影響を受ける頂点の数
		int controlPointIndicesCount = fbxCluster->GetControlPointIndicesCount();
		// 影響を受ける頂点の配列
		int* controlPointIndices = fbxCluster->GetControlPointIndices();
		double* controlPointWeights = fbxCluster->GetControlPointWeights();

		for (int j = 0; j < controlPointIndicesCount; j++) {
			int vertIndex = controlPointIndices[j];
			float weight = (float)controlPointWeights[j];
			// その他の頂点の影響を受けるボーンリストに、ボーンとウェイトのペアを追加
			weightLists[vertIndex].emplace_back(WeightSet{ (UINT)i, weight });
		}
	}
	// 頂点配列書き換え用参照
	auto& vertices = model->vertices;
	for (UINT i = 0, loopLen = vertices.size(); i < loopLen; i++) {
		// 最も大きい4つを選択
		auto& weightList = weightLists[i];
		// 降順ソート
		weightList.sort(
			[](auto const& lhs, auto const& rhs) {
				return lhs.weight > rhs.weight;
			}
		);
		int weightArrayIndex = 0;
		// 降順ソート済みのウェイトリストから
		for (auto& WeightSet : weightList) {
			// 頂点データに書き込み
			vertices[i].boneIndex[weightArrayIndex] = WeightSet.index;
			vertices[i].boneWeight[weightArrayIndex] = WeightSet.weight;
			// 4つに達したら終了
			if (++weightArrayIndex >= FbxModel::MAX_BONE_INDICES) {
				float weight = 0.f;
				// 2番目以降のウェイトを合計
				for (int j = 1; j < FbxModel::MAX_BONE_INDICES; j++) {
					weight += vertices[i].boneWeight[j];
				}
				// 合計で1(100%)になるよう調整
				vertices[i].boneWeight[0] = 1.f - weight;
				break;
			}
		}
	}
}

void FbxLoader::loadTexture(FbxModel* model, const std::string& fullpath) {
	HRESULT result = S_FALSE;

	TexMetadata& metadata = model->metadata;
	ScratchImage& scratchImg = model->scratchImg;

	constexpr size_t wfilepathLen = 128;
	wchar_t wfilepath[wfilepathLen];
	MultiByteToWideChar(CP_ACP, 0, fullpath.c_str(), -1,
						wfilepath, wfilepathLen);
	result = LoadFromWICFile(wfilepath,
							 WIC_FLAGS_NONE,
							 &metadata,
							 scratchImg);
	if (FAILED(result)) {
		assert(0);
	}
}