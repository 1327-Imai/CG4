#include "Model.h"

void Model::CreateBuffers(ID3D12Device* device) {

	HRESULT result;
	//頂点データのサイズ
	UINT sizeVB = static_cast<UINT>(sizeof(VertecxPosNormalUv) * vertices.size());

	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) ,
		D3D12_HEAP_FLAG_NONE ,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB) ,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&vertBuff)
	);

	//頂点バッファへ転送
	VertecxPosNormalUv* vertMap = nullptr;
	result = vertBuff->Map(0 , nullptr , (void**)&vertMap);
	if (SUCCEEDED(result)) {
		std::copy(vertices.begin() , vertices.end() , vertMap);
		vertBuff->Unmap(0 , nullptr);
	}

	//VBVの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	//頂点インデックス全体のサイズ
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	//インデックスバッファ生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) ,
		D3D12_HEAP_FLAG_NONE ,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB) ,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&indexBuff)
	);

	//インデックスバッファの転送
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0 , nullptr , (void**)&indexMap);
	if (SUCCEEDED(result)) {
		std::copy(indices.begin() , indices.end() , indexMap);
		indexBuff->Unmap(0 , nullptr);
	}

	//IBVの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16G16_UINT;
	ibView.SizeInBytes = sizeIB;

	const DirectX::Image* img = scratchImg.GetImage(0 , 0 , 0);
	assert(img);

	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format ,
		metadata.width ,
		(UINT)metadata.height ,
		(UINT16)metadata.arraySize ,
		(UINT16)metadata.mipLevels
	);

	//テクスチャ用バッファの生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK ,
		D3D12_MEMORY_POOL_L0) ,
		D3D12_HEAP_FLAG_NONE ,
		&texresDesc ,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&texBuff)
	);

	result = texBuff->WriteToSubresource(
		0 ,
		nullptr ,
		img->pixels ,
		(UINT)img->rowPitch ,
		(UINT)img->slicePitch
	);

	//SRv用デスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC descHeapdesc = {};
	descHeapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapdesc.NumDescriptors = 1;
	result = device->CreateDescriptorHeap(&descHeapdesc , IID_PPV_ARGS(&descHeapSrv));

	//SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	D3D12_RESOURCE_DESC resDesc = texBuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texBuff.Get() ,
									 &srvDesc ,
									 descHeapSrv->GetCPUDescriptorHandleForHeapStart()
	);

}
