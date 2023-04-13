#pragma once
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include "SimpleShader.h"
class Material
{
private:
	DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(0.0f,0.0f,0.0f,0.0f);
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> SRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
public:
	Material(DirectX::XMFLOAT4, std::shared_ptr<SimpleVertexShader>, std::shared_ptr<SimplePixelShader>);
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	void SetColorTint(DirectX::XMFLOAT4);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader>);
	void PixelShader(std::shared_ptr<SimplePixelShader>);
	void AddTextureSRV(std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>);
	void AddSampler(std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>);
	void PrepareMaterial();
};

