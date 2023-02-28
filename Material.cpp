#include "Material.h"

using namespace DirectX;
using namespace std;

Material::Material(XMFLOAT4 colorTint, shared_ptr<SimpleVertexShader> vertexShader, shared_ptr<SimplePixelShader> pixelShader, float roughness, XMFLOAT3 ambience)
{
	this->colorTint = colorTint;
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->roughness = roughness;
	this->ambience = ambience;
}

XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

float Material::GetRoughness()
{
	return roughness;
}

DirectX::XMFLOAT3 Material::GetAmbience()
{
	return ambience;
}

void Material::SetColorTint(XMFLOAT4 colorTint)
{
	this->colorTint = colorTint;
}

void Material::SetVertexShader(shared_ptr<SimpleVertexShader> vertexShader)
{
	this->vertexShader = vertexShader;
}

void Material::PixelShader(shared_ptr<SimplePixelShader> pixelShader)
{
	this->pixelShader = pixelShader;
}
