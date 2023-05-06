#pragma once
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <array>
#include "WICTextureLoader.h"
#include "DXCore.h"
#include "Ent.h"
#include "Mesh.h"
#include "Transform.h"
#include "Cam.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"
class Game: public DXCore {
	public:
		Game(HINSTANCE hInstance);
		~Game();
		void Init();
		void OnResize();
		void Update(float deltaTime, float totalTime);
		void Draw(float deltaTime, float totalTime);

	private:
		void ResetRenderTarget();
		void AddTextures(std::shared_ptr<Material>, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*);
		void Node(const char*, Ent*);
		void LightNode(const char*, Light*);
		Light MakeDir(DirectX::XMFLOAT3, DirectX::XMFLOAT3, float);
		Light MakePoint(float, DirectX::XMFLOAT3, float, DirectX::XMFLOAT3);
		Light MakeSpot(DirectX::XMFLOAT3, float, DirectX::XMFLOAT3, float, DirectX::XMFLOAT3, float);
		std::shared_ptr<SimpleVertexShader> vs;
		std::shared_ptr<SimplePixelShader> ps;
		Sky sky;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDSS;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRS;
		std::shared_ptr<Mesh> skyMesh;
		std::shared_ptr<SimpleVertexShader> skyVS;
		std::shared_ptr<SimplePixelShader> skyPS;
		Light dir;
		Light pt;
		Light spot;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
		std::vector<std::shared_ptr<Material>> mats;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> ss;
		std::vector<Ent> ents;
		Ent floor[15][15];
		std::vector<std::shared_ptr<Cam>> cams;
		int activeCam;
		int ent6Dir;
		int ent4Dir;

		// Shadows
		std::shared_ptr<SimpleVertexShader> shadowVS;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
		DirectX::XMFLOAT4X4 shadowViewMatrix;
		DirectX::XMFLOAT4X4 shadowProjectionMatrix;
		int shadowMapResolution; // should be a power of 2

		// Resources that are shared among all post processes
		Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
		std::shared_ptr<SimpleVertexShader> ppVS;
		// Resources that are tied to a particular post process
		std::shared_ptr<SimplePixelShader> ppPS;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV; // For rendering
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV; // For sampling
		int blurRadius;
};