#pragma once

#include "fbxsdk.h"
#include "model.h"
#include <string>

#include <d3d12.h>
#include <d3dx12.h>

class FbxLoader
{
private://エイリアス
	//stdを省略
	using string = std::string;

public://定数
	//モデル格納ルートパス
	static const string baseDirectory;

public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>インスタンス</returns>
	static FbxLoader* GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="device"></param>
	void Initialize(ID3D12Device* device);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// ファイルからFBXモデル読み込み
	/// </summary>
	/// <param name="modelName"></param>
	void LoadModelFromFile(const string& modelName);

	void ParseNodeRecursive(Model* model , FbxNode* fbxNode,Model::Node* parent = nullptr);

private://メンバ変数
	//デバイス
	ID3D12Device* device = nullptr;

	//FBXマネージャ
	FbxManager* fbxManager = nullptr;

	//FBXインポータ
	FbxImporter* fbxImporter = nullptr;

private:
	// privateなコンストラクタ（シングルトンパターン）
	FbxLoader() = default;
	// privateなデストラクタ（シングルトンパターン）
	~FbxLoader() = default;
	// コピーコンストラクタを禁止（シングルトンパターン）
	FbxLoader(const FbxLoader& obj) = delete;
	// コピー代入演算子を禁止（シングルトンパターン）
	void operator=(const FbxLoader& obj) = delete;
};