#pragma once
#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>

#include <string>
#include <vector>
#include <DirectXMath.h>
#include <DirectXTex.h>

#include <fbxsdk.h>

class Model
{
public:
	~Model();

public:
	//�t�����h�N���X
	friend class FbxLoader;

private://�G�C���A�X
	//Microsoft::WRL::���ȗ�
	template <class T> using ComPtr =
		Microsoft::WRL::ComPtr<T>;

	//DirectX::���ȗ�
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	using TexMetadata = DirectX::TexMetadata;
	using ScratchImage = DirectX::ScratchImage;

	//std::���ȗ�
	using string = std::string;
	template <class T> using vector = std::vector<T>;
public://�萔
	//�{�[���C���f�b�N�X�̍ő吔
	static const int MAX_BONE_INDICES = 4;


public://�T�u�N���X
	struct VertecxPosNormalUvSkin
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		UINT boneIndex[MAX_BONE_INDICES];
		float boneWeight[MAX_BONE_INDICES];
	};

	//�\����
public:
	struct Node
	{
		//���O
		std::string name;
		//���[�J���X�P�[��
		DirectX::XMVECTOR scaling = {1 , 1 , 1 , 0};
		//���[�J����]�p
		DirectX::XMVECTOR rotation = {0 , 0 , 0 , 0};
		//���[�J���ړ�
		DirectX::XMVECTOR translation = {0 , 0 , 0 , 1};
		//���[�J���ϊ��s��
		DirectX::XMMATRIX transform;
		//�O���[�o���ϊ��s��
		DirectX::XMMATRIX gloablTransform;
		//�e�m�[�h
		Node* parent = nullptr;
	};

	struct Bone {
		//���O
		std::string name;
		//�����p���̋t�s��
		DirectX::XMMATRIX invInitialPose;
		//�N���X�^�[
		FbxCluster* fbxCluster;
		//�R���X�g���N�^
		Bone(const std::string& name) {
			this->name = name;
		}

	};

	//�����o�֐�
public:
	void Draw(ID3D12GraphicsCommandList* cmdList);

	const XMMATRIX& GetModelTransform() {
		return meshNode->gloablTransform;
	}

	vector<Bone>& GetBones() {
		return bones;
	}

	FbxScene* GetFbxScene() {
		return fbxScene;
	}

private:
	//�o�b�t�@����
	void CreateBuffers(ID3D12Device* device);

private://�����o�ϐ�
	//FBX�V�[��
	FbxScene* fbxScene = nullptr;
	//���f����
	string name;
	//�m�[�h�z��
	vector<Node> nodes;
	//�{�[���z��
	vector<Bone> bones;
	//���b�V�������m�[�h
	Node* meshNode = nullptr;
	//���_�f�[�^�z��
	vector<VertecxPosNormalUvSkin> vertices;
	//���_�C���f�b�N�X�z��
	vector<unsigned short> indices;
	//�A���r�G���g�W��
	XMFLOAT3 ambient = {1 , 1 , 1};
	//�f�B�t���[�Y�W��
	XMFLOAT3 diffuse = {1 , 1 , 1};
	//�e�N�X�`�����^�f�[�^
	TexMetadata metadata = {};
	//�X�N���b�`�C���[�W
	ScratchImage scratchImg = {};

	//���_�o�b�t�@
	ComPtr<ID3D12Resource> vertBuff;
	//�C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource> indexBuff;
	//�e�N�X�`���o�b�t�@
	ComPtr<ID3D12Resource> texBuff;
	//���_�o�b�t�@�r���[
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	//�C���f�b�N�X�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	//SRV�f�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> descHeapSrv;

};

