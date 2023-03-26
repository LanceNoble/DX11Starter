#pragma once
#include <DirectXMath.h>

/// <summary>
/// <para>Transformation class</para>
/// Gonna create a transformation hierarchy out of this class when I need to
/// </summary>
class Transform
{
private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 orientation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInverseTranspose;
public:
	Transform();
	void SetPosition(float, float, float);
	void SetOrientation(float, float, float);
	void SetOrientation(DirectX::XMFLOAT4);
	void SetScale(float, float, float);
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT4 GetOrientation();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	void MoveAbsolute(float, float, float);
	void MoveRelative(float, float, float);
	void Rotate(float, float, float);
	void RotAx(DirectX::XMFLOAT3, float);
	void Scale(float, float, float);
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();
	void UpdateMatrices();
};