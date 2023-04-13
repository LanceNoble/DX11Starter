#include "Material.h"

using namespace DirectX;
using namespace std;

Material::Material(XMFLOAT4 tint, shared_ptr<SimpleVertexShader> vs, shared_ptr<SimplePixelShader> ps)
{
	this->colorTint = tint;
	this->vs = vs;
	this->ps = ps;
}

XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vs;
}

shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return ps;
}

void Material::SetColorTint(XMFLOAT4 colorTint)
{
	this->colorTint = colorTint;
}

void Material::SetVertexShader(shared_ptr<SimpleVertexShader> vertexShader)
{
	this->vs = vertexShader;
}

void Material::PixelShader(shared_ptr<SimplePixelShader> pixelShader)
{
	this->ps = pixelShader;
}

void Material::AddTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	SRVs.insert({shaderName, srv}); 
}

void Material::AddSampler(std::string sampStateName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState)
{
	samplers.insert({sampStateName, sampState});
}

/// <summary>
/// An advanced engine might prepare materials and shaders in a Renderer class
/// </summary>
void Material::PrepareMaterial()
{
	// .first accesses the current element's key in the hashtable
	// .second accesses the current element's value in the hashtable
	for (auto& t : SRVs) ps->SetShaderResourceView(t.first.c_str(), t.second);
	for (auto& s : samplers) ps->SetSamplerState(s.first.c_str(), s.second);
}
