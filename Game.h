#pragma once
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "DXCore.h"
#include "BufferStructs.h"
#include "Entity.h"
#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
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
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		std::shared_ptr<Mesh> mesh0;
		std::shared_ptr<Mesh> mesh1;
		std::shared_ptr<Mesh> mesh2;
		//std::shared_ptr<Camera> cam;
		std::vector<std::shared_ptr<Camera>> cams;
		int activeCam = 0;
		Entity entities[5];
		int entityCount = 0;
		float timer = 0;
		float fps = 0;
		float colorTint[4] = {};
};