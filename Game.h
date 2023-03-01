#pragma once
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>
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
		Light MakeDir(float, DirectX::XMFLOAT3, DirectX::XMFLOAT3);
		Light MakePoint(float, DirectX::XMFLOAT3, float, DirectX::XMFLOAT3);
		Light MakeSpot(float, DirectX::XMFLOAT3);
		std::shared_ptr<SimpleVertexShader> vs;
		std::shared_ptr<SimplePixelShader> pixelShader;
		std::shared_ptr<SimplePixelShader> ps;
		std::shared_ptr<Material> mat0;
		std::shared_ptr<Material> mat1;
		std::shared_ptr<Material> mat2;
		std::shared_ptr<Mesh> mesh0;
		std::shared_ptr<Mesh> mesh1;
		std::shared_ptr<Mesh> mesh2;
		std::shared_ptr<Mesh> mesh3;
		std::shared_ptr<Mesh> mesh4;
		std::shared_ptr<Mesh> mesh5;
		std::shared_ptr<Mesh> mesh6;
		float orientation0[3] = { 0.0f,0.0f,0.0f };
		float orientation1[3] = { 0.0f,0.0f,0.0f };
		float orientation2[3] = { 0.0f,0.0f,0.0f };
		std::vector<std::shared_ptr<Camera>> cams;
		int activeCam = 0;
		Entity entities[7];
		int entityCount = 0;
		float timer = 0;
		float fps = 0;
		Light directionalLight1;
		Light dirLight2;
		Light dirLight3;
		Light point0;
		Light point1;
};