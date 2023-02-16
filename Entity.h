#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"
#include "Camera.h"
#include <memory>
class Entity
{
private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
public:
	Entity();
	Entity(std::shared_ptr<Mesh> mesh);

	// Note: in general, a class should not delete an object it didn't make
	~Entity();

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();
	void Draw(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, float* colorTint, std::shared_ptr<Camera> cam);
};

