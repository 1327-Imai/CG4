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
protected://�G�C���A�X
	//Microsoft::WRL::���ȗ�
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//DirectX::���ȗ�
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

public://�萔
	//�{�[���̍ő吔
	static const int MAX_BONES = 32;

public://�T�u�N���X
	//�萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferDataTransform {
		XMMATRIX viewProjection;
		XMMATRIX matWorld;
		XMFLOAT3 cameraPos;
	};
	//�萔�o�b�t�@�p�f�[�^�\����
	struct ConstBufferDataSkin {
		XMMATRIX bones[MAX_BONES];
	};


public://�ÓI�����o�֐�
	//�Z�b�^�[
	static void SetDeivce(ID3D12Device* device) {
		Object3d::device = device;
	}
	static void SetCamera(Camera* camera) {
		Object3d::camera = camera;
	}

	static void CreateGraphicsPipeline();

public://�����o�֐�
	//������
	void Initialize();

	//�X�V
	void Update();

	void Draw(ID3D12GraphicsCommandList* cmdList);

	//�A�N�Z�b�T
	void SetModel(Model* model) {
		this->model = model;
	}

	//�A�j���[�V�����J�n
	void PlayAnimation();

	//�A�j���[�V������~
	void StopAnimation();

	//�A�j���[�V�����Đ��󋵐؂�ւ�
	void ToggleAnimation();

protected://�����o�ϐ�
	ComPtr<ID3D12Resource> constBufferTransform;
	ComPtr<ID3D12Resource> constBufferSkin;

	XMFLOAT3 scale = {1 , 1 , 1};
	XMFLOAT3 rotation = {0 , 0 , 0};
	XMFLOAT3 position = {0 , 0 , 0};
	XMMATRIX matWorld;

	Model* model = nullptr;

	//FBX�A�j���[�V�����p�ϐ�
	//1�t���[���̎���
	FbxTime frameTime;
	//�A�j���[�V�����J�n����
	FbxTime startTime;
	//�A�j���[�V�����I������
	FbxTime endTime;
	//���ݎ���(�A�j���[�V����)
	FbxTime currentTime;
	//�A�j���[�V�����Đ��t���O
	bool isPlay = false;

private://�ÓI�����o�ϐ�
	static ID3D12Device* device;
	static Camera* camera;
	//���[�g�V�O�l�`��
	static ComPtr<ID3D12RootSignature> rootsignature;
	//�p�C�v���C���X�e�[�g
	static ComPtr<ID3D12PipelineState> pipelinestate;
};

