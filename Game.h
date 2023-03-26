#pragma once
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>
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
		void LoadShaders(); 
		void CreateGeometry();
		void Node(const char*, Ent*);
		void LightNode(const char*, Light*);
		Light MakeDir(DirectX::XMFLOAT3, DirectX::XMFLOAT3, float);
		Light MakePoint(float, DirectX::XMFLOAT3, float, DirectX::XMFLOAT3);
		Light MakeSpot(DirectX::XMFLOAT3, float, DirectX::XMFLOAT3, float, DirectX::XMFLOAT3, float);

		// Shaders for objects in our game world
		std::shared_ptr<SimpleVertexShader> vs;
		std::shared_ptr<SimplePixelShader> ps;

		// Sky
		Sky sky;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> skySS;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDSS;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRS;
		std::shared_ptr<Mesh> skyMesh;
		std::shared_ptr<SimpleVertexShader> skyVS;
		std::shared_ptr<SimplePixelShader> skyPS;

		// Lights
		Light dir;
		Light pt;
		Light spot;

		// Meshes
		std::vector<std::shared_ptr<Mesh>> meshes;

		// Materials
		std::vector<std::shared_ptr<Material>> mats;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSurf0;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSurf1;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvNorm0;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvNorm1;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState;
		//std::shared_ptr<Material> mats[3];

		// Entities
		std::vector<Ent> ents;

		// Cameras
		std::vector<std::shared_ptr<Cam>> cams;
		int activeCam;
};