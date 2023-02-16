#pragma once
#include <DirectXMath.h>
#include "Transform.h"
class Camera
{
private:
	

	DirectX::XMFLOAT4X4 viewMat;
	DirectX::XMFLOAT4X4 projMat;
	DirectX::XMFLOAT4 initRot;
	DirectX::XMFLOAT3 initPos;
	
	
	float nearClip;
	float farClip;
	float moveSp;
	float lookSp;
	float isPerspective;
public:
	Transform transform;
	float fov;
	float aspRat;
	Camera(float aspRat, DirectX::XMFLOAT3 initPos, float fov);
	DirectX::XMFLOAT4X4 GetViewMat();
	DirectX::XMFLOAT4X4 GetProjMat();
	void UpdateProjectionMatrix(float aspRat);
	void UpdateViewMatrix();
	void Update(float dt);
};

