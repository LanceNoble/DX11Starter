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
#include "SimpleShader.h"
#include "Material.h"
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
		std::shared_ptr<SimpleVertexShader> vertexShader;
		std::shared_ptr<SimplePixelShader> pixelShader;
		std::shared_ptr<Material> mat0;
		std::shared_ptr<Material> mat1;
		std::shared_ptr<Material> mat2;
		std::shared_ptr<Mesh> mesh0;
		std::shared_ptr<Mesh> mesh1;
		std::shared_ptr<Mesh> mesh2;
		std::vector<std::shared_ptr<Camera>> cams;
		int activeCam = 0;
		Entity entities[5];
		int entityCount = 0;
		float timer = 0;
		float fps = 0;
};