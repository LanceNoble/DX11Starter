#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
class Material
{
private:
	DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(0.0f,0.0f,0.0f,0.0f);
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	float roughness;
	DirectX::XMFLOAT3 ambience;
public:
	Material(DirectX::XMFLOAT4, std::shared_ptr<SimpleVertexShader>, std::shared_ptr<SimplePixelShader>, float, DirectX::XMFLOAT3);
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	float GetRoughness();
	DirectX::XMFLOAT3 GetAmbience();
	void SetColorTint(DirectX::XMFLOAT4 colorTint);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void PixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
};

