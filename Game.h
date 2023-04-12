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
		std::vector<std::shared_ptr<Cam>> cams;
		int activeCam;
};