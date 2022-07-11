cbuffer cbuff0 : register(b0) {
	matrix viewproj;	// �r���[�v���W�F�N�V�����s��
	matrix world;		// ���[���h�s��	
	float3 cameraPos;	// �J�������W(���[���h���W)
};

cbuffer cbuff1 : register(b1) {
	float3 m_ambient : packoffset(c0);  // �A���r�G���g�W��
	float3 m_diffuse : packoffset(c1);  // �f�B�t���[�Y�W��
	float3 m_specular : packoffset(c2); // �f�B�t���[�Y�W��
	float m_alpha : packoffset(c2.w);   // �A���t�@
}

cbuffer cbuff2 : register(b2) {
	float3 lightPos;   // ���C�g�ւ̕����̒P�ʃx�N�g��
	float3 lightColor;  // ���C�g�̐F(RGB)
};

// �{�[���̍ő吔(FbxObj3d.h�̒萔�ƍ��킹��)
static const int MAX_BONES = 32;

// �{�[���̃X�L�j���O�s�񂪓���
cbuffer skinning : register(b3) {
	matrix matSkinning[MAX_BONES];
}

struct VSInput {
	float4 pos : POSITION;	// �ʒu
	float3 normal : NORMAL;	// ���_�̖@��
	float2 uv : TEXCOORD;	// �e�N�X�`���\���W
	uint4 boneIndices : BONEINDICES;	// �{�[���̔ԍ�
	float4 boneWeights : BONEWEIGHTS;	// �{�[���̃X�L���E�F�C�g
};

struct VSOutput {
	float4 svpos : SV_POSITION;	// �V�X�e���p���_���W
	float4 worldPos : POSITION;
	float3 normal : NORMAL;		// �@��
	float2 uv : TEXCOORD;		// uv
};

// �����_�[�^�[�Q�b�g�̐���2��
struct PSOutput {
	float4 target0 : SV_TARGET0;
	float4 target1 : SV_TARGET1;
};