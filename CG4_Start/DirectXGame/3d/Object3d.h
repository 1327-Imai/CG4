#pragma once

#include "Model.h"
#include "Camera.h"

#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <string>

#include "FbxLoader.h"

class Object3d
{
protected://エイリアス
	//Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public://定数
	//ボーンの最大数
	static const int MAX_BONES = 32;

public://サブクラス
	//定数バッファ用データ構造体
	struct ConstBufferDataTransform {
		XMMATRIX viewProjection;
		XMMATRIX matWorld;
		XMFLOAT3 cameraPos;
	};
	//定数バッファ用データ構造体
	struct ConstBufferDataSkin {
		XMMATRIX bones[MAX_BONES];
	};


public://静的メンバ関数
	//セッター
	static void SetDeivce(ID3D12Device* device) {
		Object3d::device = device;
	}
	static void SetCamera(Camera* camera) {
		Object3d::camera = camera;
	}

	static void CreateGraphicsPipeline();

public://メンバ関数
	//初期化
	void Initialize();

	//更新
	void Update();

	void Draw(ID3D12GraphicsCommandList* cmdList);

	//アクセッサ
	void SetModel(Model* model) {
		this->model = model;
	}

	//アニメーション開始
	void PlayAnimation();

	//アニメーション停止
	void StopAnimation();

	//アニメーション再生状況切り替え
	void ToggleAnimation();

protected://メンバ変数
	ComPtr<ID3D12Resource> constBufferTransform;
	ComPtr<ID3D12Resource> constBufferSkin;

	XMFLOAT3 scale = {1 , 1 , 1};
	XMFLOAT3 rotation = {0 , 0 , 0};
	XMFLOAT3 position = {0 , 0 , 0};
	XMMATRIX matWorld;

	Model* model = nullptr;

	//FBXアニメーション用変数
	//1フレームの時間
	FbxTime frameTime;
	//アニメーション開始時間
	FbxTime startTime;
	//アニメーション終了時間
	FbxTime endTime;
	//現在時間(アニメーション)
	FbxTime currentTime;
	//アニメーション再生フラグ
	bool isPlay = false;

private://静的メンバ変数
	static ID3D12Device* device;
	static Camera* camera;
	//ルートシグネチャ
	static ComPtr<ID3D12RootSignature> rootsignature;
	//パイプラインステート
	static ComPtr<ID3D12PipelineState> pipelinestate;
};

