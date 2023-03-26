#pragma once
#include <DirectXMath.h>
#include "Transform.h"

/// <summary>
/// Camera class
/// </summary>
class Cam
{
private:
	Transform tf;
	float aspRat;
	float fov;
	float nearClip;
	float farClip;
	float moveSp;
	float lookSpX;
	float lookSpY;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
public:
	Cam(float, DirectX::XMFLOAT3 = DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3 = DirectX::XMFLOAT3(0,0,0), float = 90, float = 0.1f, float = 900, float = 2, float = 0.001f, float = 0.001f);
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProj();
	DirectX::XMFLOAT3 GetPos();
	float GetFOV();
	float GetAspRat();
	void UpdateProj(float);
	void Move(float);
};

