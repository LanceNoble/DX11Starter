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

void Material::AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({shaderName, srv}); 
	//pixelShader->SetShaderResourceView(shaderName, srv);
}

void Material::AddSampler(std::string sampStateName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState)
{
	samplers.insert({sampStateName, sampState});
	//pixelShader->SetSamplerState(sampStateName, sampState);
}

void Material::PrepareMaterial()
{
	// .first accesses the current element's key in the hashtable
	// .second accesses the current element's value in the hashtable
	for (auto& t : textureSRVs) { 
		//if (srv == t.second) {
			pixelShader->SetShaderResourceView(t.first.c_str(), t.second);
		//}
		 
	}
	for (auto& s : samplers) { 
		//if (sampState == s.second) {
			pixelShader->SetSamplerState(s.first.c_str(), s.second);
		//}
	}
}
