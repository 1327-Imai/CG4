#pragma once
#include <string>
#include <vector>
#include <DirectXMath.h>

class Model
{
public:
	//�t�����h�N���X
	friend class FbxLoader;

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

	//�����o�֐�

private:
	//���f����
	std::string name;
	//�m�[�h�z��
	std::vector<Node> nodes;

};
