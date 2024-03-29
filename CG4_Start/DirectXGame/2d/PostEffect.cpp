#include "PostEffect.h"

#include <d3dx12.h>
#include <d3dcompiler.h>
#include "WinApp.h"
#include "input.h"

#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;

//静的メンバ変数の実態
const float PostEffect::clearColor[4] = {0.25 , 0.5f , 0.1f , 0.0f};

PostEffect::PostEffect() : Sprite(
	100 ,
	{0 , 0} ,
	{500.0f , 500.0f} ,
	{1 , 1 , 1 , 1} ,
	{0.0f , 0.0f} ,
	false ,
	false) {

}

void PostEffect::Initialize() {
	HRESULT result;

	//頂点バッファの生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) ,
		D3D12_HEAP_FLAG_NONE ,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPosUv) * vertNum) ,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&vertBuff)
	);
	assert(SUCCEEDED(result));

	VertexPosUv vertices[vertNum] = {
		{{-0.5f , -0.5f , 0.0f} , {0.0f , 1.0f}} ,
		{{-0.5f , +0.5f , 0.0f} , {0.0f , 0.0f}} ,
		{{+0.5f , -0.5f , 0.0f} , {1.0f , 1.0f}} ,
		{{+0.5f , +0.5f , 0.0f} , {1.0f , 0.0f}} ,
	};

	VertexPosUv* vertMap = nullptr;
	result = vertBuff->Map(0 , nullptr , (void**)&vertMap);
	if (SUCCEEDED(result)) {
		memcpy(vertMap , vertices , sizeof(vertices));
		vertBuff->Unmap(0 , nullptr);
	}

	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(VertexPosUv) * 4;
	vbView.StrideInBytes = sizeof(VertexPosUv);

	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) ,
		D3D12_HEAP_FLAG_NONE ,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff) ,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&constBuff)
	);
	assert(SUCCEEDED(result));

	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ,
		WinApp::window_width ,
		(UINT)WinApp::window_height ,
		1 , 0 , 1 , 0 , D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	for (int i = 0; i < 2; i++) {
		result = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK ,
			D3D12_MEMORY_POOL_L0) ,
			D3D12_HEAP_FLAG_NONE ,
			&texresDesc ,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
			&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB , clearColor) ,
			IID_PPV_ARGS(&texBuff[i])
		);
		assert(SUCCEEDED(result));

		{//テクスチャを赤クリア
	//画素数(1,280 x 720 = 921,600ピクセル)
			const UINT pixelCount = WinApp::window_width * WinApp::window_height;
			//画像1行分のデータサイズ
			const UINT rowPitch = sizeof(UINT) * WinApp::window_width;
			//画像全体のデータサイズ
			const UINT depthPitch = rowPitch * WinApp::window_height;
			//画像イメージ
			UINT* img = new UINT[pixelCount];
			for (int j = 0; j < pixelCount; j++) {
				img[j] = 0xff0000ff;
			}

			//テクスチャバッファにデータ転送
			result = texBuff[i]->WriteToSubresource(
				0 , nullptr , img , rowPitch , depthPitch);
			assert(SUCCEEDED(result));
			delete[] img;
		}
	}

	//SRV用デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC srvDescHeapDesc = {};
	srvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescHeapDesc.NumDescriptors = 2;
	//SRV用デスクリプタヒープ生成
	result = device->CreateDescriptorHeap(
		&srvDescHeapDesc , IID_PPV_ARGS(&descHeapSRV)
	);
	assert(SUCCEEDED(result));

	//SRV設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < 2; i++) {
		//デスクリプタヒープにSRV作成
		device->CreateShaderResourceView(
			texBuff[i].Get() ,
			&srvDesc ,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
			descHeapSRV->GetCPUDescriptorHandleForHeapStart() , i ,
			device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
		);
	}

	//RTV用デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc = {};
	rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHeapDesc.NumDescriptors = 2;
	//RTV用デスクリプタヒープ生成
	result = device->CreateDescriptorHeap(
		&rtvDescHeapDesc , IID_PPV_ARGS(&descHeapRTV)
	);
	assert(SUCCEEDED(result));

	//RTV設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int i = 0; i < 2; i++) {
		//デスクリプタヒープにRTV作成
		device->CreateRenderTargetView(
			texBuff[i].Get() ,
			&rtvDesc ,
			CD3DX12_CPU_DESCRIPTOR_HANDLE(
			descHeapRTV->GetCPUDescriptorHandleForHeapStart() ,
			i ,
			device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
		);
	}

	//深度バッファ
	CD3DX12_RESOURCE_DESC depthResDesc =
		CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT ,
		WinApp::window_width ,
		WinApp::window_height ,
		1 , 0 ,
		1 , 0 ,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);

	//深度バッファの生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) ,
		D3D12_HEAP_FLAG_NONE ,
		&depthResDesc ,
		D3D12_RESOURCE_STATE_DEPTH_WRITE ,
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT , 1.0f , 0) ,
		IID_PPV_ARGS(&depthBuff)
	);
	assert(SUCCEEDED(result));

	//DSV用デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescHeapDesc = {};
	dsvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescHeapDesc.NumDescriptors = 1;
	//DSV用デスクリプタヒープ生成
	result = device->CreateDescriptorHeap(
		&dsvDescHeapDesc , IID_PPV_ARGS(&descHeapDSV)
	);
	assert(SUCCEEDED(result));

	//DSV設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	//デスクリプタヒープにDSV作成
	device->CreateDepthStencilView(
		depthBuff.Get() ,
		&dsvDesc ,
		descHeapDSV->GetCPUDescriptorHandleForHeapStart()
	);

	//グラフィックスパイプラインの生成
	CreateGraphicsPipelineState();
}

void PostEffect::Draw(ID3D12GraphicsCommandList* cmdList) {

	//定数バッファにデータ転送
	ConstBufferData* constMap = nullptr;
	HRESULT result = this->constBuff->Map(0 , nullptr , (void**)&constMap);
	if (SUCCEEDED(result)) {
		constMap->color = this->color;
		constMap->mat = XMMatrixIdentity();
		this->constBuff->Unmap(0 , nullptr);
	}

	//パイプラインステートの設定
	cmdList->SetPipelineState(pipelineState.Get());
	//ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(rootSignature.Get());
	//プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//頂点バッファの設定
	cmdList->IASetVertexBuffers(0 , 1 , &this->vbView);

	ID3D12DescriptorHeap* ppHeaps[] = {descHeapSRV.Get()};
	//デスクリプタヒープをセット
	cmdList->SetDescriptorHeaps(_countof(ppHeaps) , ppHeaps);
	//定数バッファビューをセット
	cmdList->SetGraphicsRootConstantBufferView(0 , this->constBuff->GetGPUVirtualAddress());

	//シェーダリソースビューをセット
	cmdList->SetGraphicsRootDescriptorTable(
		1 ,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(
		descHeapSRV->GetGPUDescriptorHandleForHeapStart() , 0 ,
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	);

	cmdList->SetGraphicsRootDescriptorTable(
		2 ,
		CD3DX12_GPU_DESCRIPTOR_HANDLE(
		descHeapSRV->GetGPUDescriptorHandleForHeapStart() , 1 ,
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
	);



	//描画コマンド
	cmdList->DrawInstanced(4 , 1 , 0 , 0);
}

void PostEffect::PreDrawScene(ID3D12GraphicsCommandList* cmdList) {

	for (int i = 0; i < 2; i++) {
		//リソースバリアを集中(シェーダーリソース→描画可能)
		cmdList->ResourceBarrier(
			1 ,
			&CD3DX12_RESOURCE_BARRIER::Transition(texBuff[i].Get() ,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ,
			D3D12_RESOURCE_STATE_RENDER_TARGET)
		);
	}

	//レンダーターゲットビュー用デスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHs[2];
	for (int i = 0; i < 2; i++) {
		rtvHs[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			descHeapRTV->GetCPUDescriptorHandleForHeapStart() ,
			i ,
			device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	}

	//深度ステンシルビュー用デスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH =
		descHeapDSV->GetCPUDescriptorHandleForHeapStart();

	cmdList->OMSetRenderTargets(2 , rtvHs , false , &dsvH);

	CD3DX12_VIEWPORT viewports[2];
	CD3DX12_RECT scissorRects[2];

	for (int i = 0; i < 2; i++) {
		viewports[i] = CD3DX12_VIEWPORT(
			0.0f , 0.0f , WinApp::window_width , WinApp::window_height);
		scissorRects[i] = CD3DX12_RECT(
			0.0f , 0.0f , WinApp::window_width , WinApp::window_height);
	}

	//ビューポートの設定
	cmdList->RSSetViewports(2 , viewports);
	//シザリング矩形の設定
	cmdList->RSSetScissorRects(2 , scissorRects);

	for (int i = 0; i < 2; i++) {
		//全画面クリア
		cmdList->ClearRenderTargetView(rtvHs[i] , clearColor , 0 , nullptr);
	}

	//深度バッファのクリア
	cmdList->ClearDepthStencilView(dsvH , D3D12_CLEAR_FLAG_DEPTH , 1.0f , 0 , 0 , nullptr);

}

void PostEffect::PostDrawScene(ID3D12GraphicsCommandList* cmdList) {

	for (int i = 0; i < 2; i++) {
		cmdList->ResourceBarrier(
			1 , &CD3DX12_RESOURCE_BARRIER::Transition(texBuff[i].Get() ,
			D3D12_RESOURCE_STATE_RENDER_TARGET , D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE)
		);
	}
}

void PostEffect::CreateGraphicsPipelineState() {

	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; //頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;	//ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; //エラーオブジェクト

	//頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/shaders/PostEffectTestVS.hlsl" ,	//シェーダファイル名
		nullptr ,
		D3D_COMPILE_STANDARD_FILE_INCLUDE , //インクルード可能にする
		"main" , "vs_5_0" ,	//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION , //デバッグ用設定
		0 ,
		&vsBlob , &errorBlob);
	if (FAILED(result)) {
		//errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer() ,
					errorBlob->GetBufferSize() ,
					errstr.begin());
		errstr += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());

	}

	//ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"Resources/shaders/PostEffectTestPS.hlsl" ,	//シェーダファイル名
		nullptr ,
		D3D_COMPILE_STANDARD_FILE_INCLUDE , //インクルード可能にする
		"main" , "ps_5_0" ,	//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION , //デバッグ用設定
		0 ,
		&psBlob , &errorBlob);
	if (FAILED(result)) {
		//errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer() ,
					errorBlob->GetBufferSize() ,
					errstr.begin());
		errstr += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());

	}

	//頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ //xy座標(1行で書いたほうが見やすい)
			"POSITION" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 ,
			D3D12_APPEND_ALIGNED_ELEMENT ,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0
		} ,
		{ //uv座標(1行で書いたほうが見やすい)
			"TEXCOORD" , 0 , DXGI_FORMAT_R32G32_FLOAT , 0 ,
			D3D12_APPEND_ALIGNED_ELEMENT ,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0
		} ,
	};

	//グラフィックスパイプラインの流れを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	//サンプルマスク
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; //標準設定
	//ラスタライザステート
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//デプスステンシルステート
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS; //常に上書きルール

	//レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	//RBGA全てのチャンネルを描画
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	//ブレンドステートの設定
	gpipeline.BlendState.RenderTarget[0] = blenddesc;

	//深度バッファのフォーマット
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	//図形の形状設定（三角形）
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;	//描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; //0〜255指定のRGBA
	gpipeline.SampleDesc.Count = 1; //1ピクセルにつき1回サンプリング

	//デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV0;
	descRangeSRV0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV , 1 , 0); //t0 レジスタ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV1;
	descRangeSRV1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV , 1 , 1); //t0 レジスタ

	//ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0 , 0 , D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsDescriptorTable(1 , &descRangeSRV0 , D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1 , &descRangeSRV1 , D3D12_SHADER_VISIBILITY_ALL);

	//スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0 , D3D12_FILTER_MIN_MAG_MIP_POINT); //s0 レジスタ
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

	//ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams) , rootparams , 1 , &samplerDesc , D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	//バージョン自動判定のシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc , D3D_ROOT_SIGNATURE_VERSION_1_0 , &rootSigBlob , &errorBlob);
	assert(SUCCEEDED(result));
	//ルートシグネチャの生成
	result = device->CreateRootSignature(0 , rootSigBlob->GetBufferPointer() , rootSigBlob->GetBufferSize() , IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));

	gpipeline.pRootSignature = rootSignature.Get();

	//グラフィックスパイプラインの生成
	result = device->CreateGraphicsPipelineState(&gpipeline , IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));

}