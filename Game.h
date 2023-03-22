#pragma once
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "WICTextureLoader.h"
#include "DXCore.h"
#include "Entity.h"
#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"
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
		void Node(const char*, Entity*);
		void LightNode(const char*, Light*);
		Light MakeDir(float, DirectX::XMFLOAT3, DirectX::XMFLOAT3);
		Light MakePoint(float, DirectX::XMFLOAT3, float, DirectX::XMFLOAT3);
		Light MakeSpot(float, DirectX::XMFLOAT3);
		std::shared_ptr<SimpleVertexShader> vs;
		std::shared_ptr<SimplePixelShader> ps;
		std::shared_ptr<Mesh> meshes[3];
		// Each loaded texture needs a unique srv
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSurf0;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvSurf1;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvNorm0;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvNorm1;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampState;
		std::shared_ptr<Material> mats[3];
		Entity ents[9];
		Light dir0;
		Light dir1;
		Light dir2;
		Light point0;
		Light point1;
		std::vector<std::shared_ptr<Camera>> cams;
		int entCount;
		int meshCount;
		int matCount;
		int activeCam;
		bool playerControlled;
};