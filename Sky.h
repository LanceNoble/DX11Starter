#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include "Mesh.h"
#include "SimpleShader.h"
#include "WICTextureLoader.h"
#include "Cam.h"
class Sky
{
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<SimpleVertexShader> simpleVertexShader;
	std::shared_ptr<SimplePixelShader> simplePixelShader;
public:
	Sky();
	Sky(Microsoft::WRL::ComPtr<ID3D11SamplerState>, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, 
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>, 
		Microsoft::WRL::ComPtr<ID3D11RasterizerState>, 
		std::shared_ptr<Mesh>, 
		std::shared_ptr<SimpleVertexShader>, 
		std::shared_ptr<SimplePixelShader>, 
		Microsoft::WRL::ComPtr<ID3D11Device>);
	void CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext>, std::vector<std::shared_ptr<Cam>>, int);
};

