#pragma once
#include <string>
#include <vector>
#include <DirectXMath.h>

class Model
{
public:
	//フレンドクラス
	friend class FbxLoader;

	//構造体
public:
	struct Node
	{
		//名前
		std::string name;
		//ローカルスケール
		DirectX::XMVECTOR scaling = {1 , 1 , 1 , 0};
		//ローカル回転角
		DirectX::XMVECTOR rotation = {0 , 0 , 0 , 0};
		//ローカル移動
		DirectX::XMVECTOR translation = {0 , 0 , 0 , 1};
		//ローカル変換行列
		DirectX::XMMATRIX transform;
		//グローバル変換行列
		DirectX::XMMATRIX gloablTransform;
		//親ノード
		Node* parent = nullptr;
	};

	//メンバ関数

private:
	//モデル名
	std::string name;
	//ノード配列
	std::vector<Node> nodes;

};

