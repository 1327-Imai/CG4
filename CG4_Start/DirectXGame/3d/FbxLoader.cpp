#include "FbxLoader.h"
#include <cassert>

using namespace DirectX;

const std::string FbxLoader::baseDirectory = "Resources/";

FbxLoader* FbxLoader::GetInstance()
{
	static FbxLoader instance;
	return &instance;
}

void FbxLoader::Initialize(ID3D12Device* device)
{
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

void FbxLoader::Finalize()
{
	//各種インスタンスの破棄
	fbxImporter->Destroy();
	fbxManager->Destroy();
}

void FbxLoader::LoadModelFromFile(const string& modelName)
{

	//モデルと同じ生のフォルダから読み込む
	const string directoryPath = baseDirectory + modelName + "/";

	//拡張子.fbxを追加
	const string fileName = modelName + ".fbx";

	//連結してフルパスを得る
	const string fullpath = directoryPath + fileName;

	if (!fbxImporter->Initialize(fullpath.c_str() , -1 , fbxManager->GetIOSettings()))
	{
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

void FbxLoader::ParseNodeRecursive(Model* model , FbxNode* fbxNode , Model::Node* parent)
{
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

	//子ノードに対して再起呼び出し
	for (int i = 0; i < fbxNode->GetChildCount(); i++)
	{
		ParseNodeRecursive(model , fbxNode->GetChild(i),&node);
	}
}
