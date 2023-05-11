#include "FbxLoader.h"
#include <cassert>

using namespace DirectX;

const std::string FbxLoader::baseDirectory = "Resources/";
const std::string FbxLoader::defaultTextureFileName = "white1x1.png";

FbxLoader * FbxLoader::GetInstance() {
	static FbxLoader instance;
	return &instance;
}

void FbxLoader::Initialize(ID3D12Device* device) {
	//再初期化チェック
	assert(fbxManager == nullptr);

	//引数からメンバ変数に代入
	this->device = device;

	//FBXマネージャの生成
	fbxManager = FbxManager::Create();

	//FBXマネージャの入力設定
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager , IOSROOT);
	fbxManager->SetIOSettings(ios);

	//FBXインポータの生成
	fbxImporter = FbxImporter::Create(fbxManager , "");

}

void FbxLoader::Finalize() {
	//各種インスタンスの破棄
	fbxImporter->Destroy();
	fbxManager->Destroy();
}

void FbxLoader::LoadModelFromFile(const string& modelName) {

	//モデルと同じ生のフォルダから読み込む
	const string directoryPath = baseDirectory + modelName + "/";

	//拡張子.fbxを追加
	const string fileName = modelName + ".fbx";

	//連結してフルパスを得る
	const string fullpath = directoryPath + fileName;

	if (!fbxImporter->Initialize(fullpath.c_str() , -1 , fbxManager->GetIOSettings())) {
		assert(0);
	}

	//シーン生成
	FbxScene* fbxScene = FbxScene::Create(fbxManager , "fbxScene");

	//ファイルからロードしたFBXの情報をシーンにインポート
	fbxImporter->Import(fbxScene);

	//モデル生成
	Model* model = new Model;
	model->name = modelName;

	//FBXノードの数を取得
	int nodeCount = fbxScene->GetNodeCount();
	//あらかじめ必要部分のメモリを確保することでアドレスがずれるのを予防
	model->nodes.reserve(nodeCount);

	//ルートノードから順に解析してモデルに流し込む
	ParseNodeRecursive(model , fbxScene->GetRootNode());

	//FBXシーン開放
	fbxScene->Destroy();

}

void FbxLoader::ParseNodeRecursive(Model* model , FbxNode* fbxNode , Model::Node* parent) {
	//モデルにノードを追加
	model->nodes.emplace_back();
	Model::Node& node = model->nodes.back();
	//ノード名を取得
	string name = fbxNode->GetName();

	//FBXノードのローカル移動情報
	FbxDouble3 rotation = fbxNode->LclRotation.Get();
	FbxDouble3 scaling = fbxNode->LclScaling.Get();
	FbxDouble3 translation = fbxNode->LclTranslation.Get();

	//解析変換して代入
	node.rotation = {(float)rotation[0] , (float)rotation[1] , (float)rotation[2] , 0.0f};
	node.scaling = {(float)scaling[0] , (float)scaling[1] , (float)scaling[2] , 0.0f};
	node.translation = {(float)translation[0] , (float)translation[1] , (float)translation[2] , 1.0f};

	//回転角を度数法から弧度法に変換
	node.rotation.m128_f32[0] = XMConvertToRadians(node.rotation.m128_f32[0]);
	node.rotation.m128_f32[1] = XMConvertToRadians(node.rotation.m128_f32[1]);
	node.rotation.m128_f32[2] = XMConvertToRadians(node.rotation.m128_f32[2]);

	//スケール、回転、平行移動行列の計算
	XMMATRIX matScaling , matRotation , matTransration;
	matScaling = XMMatrixScalingFromVector(node.scaling);
	matRotation = XMMatrixRotationRollPitchYawFromVector(node.rotation);
	matTransration = XMMatrixTranslationFromVector(node.translation);

	//1ローカル変形行列の計算
	node.transform = XMMatrixIdentity();
	node.transform *= matScaling;
	node.transform *= matRotation;
	node.transform *= matTransration;

	//FBXノードのメッシュ情報を解析
	FbxNodeAttribute* fbxNodeAttribute = fbxNode->GetNodeAttribute();

	if (fbxNodeAttribute) {
		if (fbxNodeAttribute) {
			if (fbxNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
				model->meshNode = &node;
				ParseMesh(model , fbxNode);
			}
		}
	}

	//子ノードに対して再起呼び出し
	for (int i = 0; i < fbxNode->GetChildCount(); i++) {
		ParseNodeRecursive(model , fbxNode->GetChild(i) , &node);
	}
}

void FbxLoader::ParseMesh(Model* model , FbxNode* fbxNode) {

	//ノードのメッシュを取得
	FbxMesh* fbxMesh = fbxNode->GetMesh();

	//頂点座標読み取り
	ParseMeshVertices(model , fbxMesh);
	//面を構成するデータの読み取り
	ParseMeshFaces(model , fbxMesh);
	//マテリアルの読み取り
	ParseMaterial(model , fbxNode);

}

void FbxLoader::ParseMeshVertices(Model* model , FbxMesh* fbxMesh) {

	auto& vertices = model->vertices;

	//頂点データの数
	const int controlPointscount =
		fbxMesh->GetControlPointsCount();
	//必要数だけ頂点データ配列を確保
	Model::VertecxPosNormalUv vert{};
	model->vertices.resize(controlPointscount , vert);

	//FBXメッシュの頂点座標配列を取得
	FbxVector4* pCoord = fbxMesh->GetControlPoints();

	//FBXメッシュの全頂点座標をモデル内の配列にコピーする
	for (int i = 0; i < controlPointscount; i++) {
		Model::VertecxPosNormalUv& vertex = vertices[i];
		//座標のコピー
		vertex.pos.x = (float)pCoord[i][0];
		vertex.pos.y = (float)pCoord[i][1];
		vertex.pos.z = (float)pCoord[i][2];
	}
}

void FbxLoader::ParseMeshFaces(Model* model , FbxMesh* fbxMesh) {

	auto& vertices = model->vertices;
	auto& indices = model->indices;

	//1ファイルに複数メッシュのモデルは非対応
	assert(indices.size() == 0);
	//面の数
	const int polygonCount = fbxMesh->GetPolygonCount();
	//UVでオタの数
	const int textureUVcount = fbxMesh->GetTextureUVCount();
	//UV名リスト
	FbxStringList uvNames;
	fbxMesh->GetUVSetNames(uvNames);

	//面ごとの情報読み取り
	for (int i = 0; i < polygonCount; i++) {
		//面を構成する頂点の数を取得(3なら三角形ポリゴン)
		const int polygonSize = fbxMesh->GetPolygonSize(i);
		assert(polygonSize <= 4);

		//頂点ずつ処理
		for (int j = 0; j < polygonSize; j++) {
			//FBX頂点配列のインデックス
			int index = fbxMesh->GetPolygonVertex(i , j);
			assert(index >= 0);

			//頂点法線読み込み
			Model::VertecxPosNormalUv& vertex = vertices[index];
			FbxVector4 normal;
			if (fbxMesh->GetPolygonVertexNormal(i , j , normal)) {
				vertex.normal.x = (float)normal[0];
				vertex.normal.y = (float)normal[1];
				vertex.normal.z = (float)normal[2];
			}

			//テクスチャUV読み込み
			if (textureUVcount > 0) {
				FbxVector2 uvs;
				bool lUnmappedUv;
				//0番決め打ちで読み込み
				if (fbxMesh->GetPolygonVertexUV(i , j , uvNames[0] , uvs , lUnmappedUv)) {
					vertex.uv.x = (float)uvs[0];
					vertex.uv.y = (float)uvs[1];
				}
			}
			//インデックス配列に頂点インデックス追加
			//3頂点までなら
			if (j < 3) {
				//1点追加し、他の2点を三角形を構築する
				indices.push_back(index);
			}
			//4頂点目
			else {
				//3点追加し
				//四角形の0,1,2,3の内2,3,0で三角形を構築する
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

void FbxLoader::ParseMaterial(Model* model , FbxNode* fbxNode) {

	const int materialCount = fbxNode->GetMaterialCount();
	if (materialCount) {
		//先頭のマテリアルを取得
		FbxSurfaceMaterial* material = fbxNode->GetMaterial(0);
		//テクスチャを読み込んだかどうかを表すフラグ
		bool textureLoaded = false;

		if (material) {
			FbxSurfaceLambert* lambert =
				static_cast<FbxSurfaceLambert*>(material);

			//環境光係数
			FbxPropertyT<FbxDouble3> ambient = lambert->Ambient;
			model->ambient.x = (float)ambient.Get()[0];
			model->ambient.y = (float)ambient.Get()[1];
			model->ambient.z = (float)ambient.Get()[2];
		
			//拡散反射光係数
			FbxPropertyT<FbxDouble3> diffuse = lambert->Diffuse;
			model->diffuse.x = (float)diffuse.Get()[0];
			model->diffuse.y = (float)diffuse.Get()[1];
			model->diffuse.z = (float)diffuse.Get()[2];

			//ディフューズテクスチャを作り出す
			const FbxProperty diffuseProperty =
				material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if (diffuseProperty.IsValid()) {
				const FbxFileTexture* texture =
					diffuseProperty.GetSrcObject<FbxFileTexture>();
				
				if (texture) {
					const char* filepath = texture->GetFileName();

					//ファイルパスからファイル名を抽出
					string path_str(filepath);
					string name = ExtractFileName(path_str);

					//テクスチャ読み込み
					LoadTexture(model , baseDirectory + model->name + "/" + name);
					textureLoaded = true;
				}
			}
		
		}

		//テクスチャがない場合は標準テクスチャを貼る
		if (!textureLoaded) {
			LoadTexture(model , baseDirectory + defaultTextureFileName);
		}

	}
}

void FbxLoader::LoadTexture(Model* model , const std::string& fullpath) {

	HRESULT result = S_FALSE;
	//WICテクスチャのロード
	TexMetadata& metadata = model->metadata;
	ScratchImage& scratchImg = model->scratchImg;
	//ユニコード文字列に変換
	wchar_t wfilepath[128];
	MultiByteToWideChar(CP_ACP , 0 , fullpath.c_str() , -1 , wfilepath , _countof(wfilepath));
	result = LoadFromWICFile(
		wfilepath ,
		WIC_FLAGS_NONE ,
		&metadata ,
		scratchImg
	);
	if (FAILED(result)) {
		assert(0);
	}
}

std::string FbxLoader::ExtractFileName(const std::string& path) {
	size_t pos1;
	//区切り文字'\\'が出てくる一番最後の部分を検索
	pos1 = path.rfind('\\');
	if (pos1 != string::npos) {
		return path.substr(pos1 + 1 , path.size() - 1);
	}
	//区切り文字'/'が出てくる一番最後の部分を検索
	pos1 = path.rfind('/');
	if (pos1 != string::npos) {
		return path.substr(pos1 + 1 , path.size() - 1);
	}

	return path;
}
