#include "Model.h"

Model::~Model(){
	//FBX�V�[���̉��
	fbxScene->Destroy();
}

void Model::Draw(ID3D12GraphicsCommandList* cmdList){
	//���_�o�b�t�@���Z�b�g
	cmdList->IASetVertexBuffers(0 , 1 , &vbView);
	//�C���f�b�N�X�o�b�t�@���Z�b�g
	cmdList->IASetIndexBuffer(&ibView);
	//�f�X�N���v�^�q�[�v�̃Z�b�g
	ID3D12DescriptorHeap* ppHeaps[] = {descHeapSrv.Get()};
	cmdList->SetDescriptorHeaps(_countof(ppHeaps) , ppHeaps);
	//�V�F�[�_�[���\�[�X�r���[���Z�b�g
	cmdList->SetGraphicsRootDescriptorTable(1 , descHeapSrv->GetGPUDescriptorHandleForHeapStart());

	//�`��R�}���h
	cmdList->DrawIndexedInstanced((UINT)indices.size() , 1 , 0 , 0 , 0);

}

void Model::CreateBuffers(ID3D12Device* device)
{

	//���_�f�[�^�S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�f�[�^�̗v�f��
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * vertices.size());

	HRESULT result;
	//���_�o�b�t�@�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	result = device->CreateCommittedResource(
		&heapProp ,
		D3D12_HEAP_FLAG_NONE ,
		&resDesc ,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&vertBuff)
	);

	assert(SUCCEEDED(result));

	//���_�o�b�t�@�֓]��
	VertecxPosNormalUvSkin* vertMap = nullptr;
	result = vertBuff->Map(0 , nullptr , (void**)&vertMap);
	if (SUCCEEDED(result)) {
		std::copy(vertices.begin() , vertices.end() , vertMap);
		vertBuff->Unmap(0 , nullptr);
	}

	//VBV�̍쐬
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	//���_�C���f�b�N�X�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	//�C���f�b�N�X�o�b�t�@����
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) ,
		D3D12_HEAP_FLAG_NONE ,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB) ,
		D3D12_RESOURCE_STATE_GENERIC_READ ,
		nullptr ,
		IID_PPV_ARGS(&indexBuff)
	);

	//�C���f�b�N�X�o�b�t�@�̓]��
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0 , nullptr , (void**)&indexMap);
	if (SUCCEEDED(result)) {
		std::copy(indices.begin() , indices.end() , indexMap);
		indexBuff->Unmap(0 , nullptr);
	}

	//IBV�̍쐬
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
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

	//�e�N�X�`���p�o�b�t�@�̐���
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

	//SRV�p�f�X�N���v�^�q�[�v�𐶐�
	D3D12_DESCRIPTOR_HEAP_DESC descHeapdesc = {};
	descHeapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapdesc.NumDescriptors = 1;
	result = device->CreateDescriptorHeap(&descHeapdesc , IID_PPV_ARGS(&descHeapSrv));

	//SRV�̍쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	D3D12_RESOURCE_DESC texResDesc = texBuff->GetDesc();

	srvDesc.Format = texResDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texBuff.Get() ,
									 &srvDesc ,
									 descHeapSrv->GetCPUDescriptorHandleForHeapStart()
	);

}