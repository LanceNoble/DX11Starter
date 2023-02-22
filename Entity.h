#pragma once
#include "Transform.h"
#include "Mesh.h"

#include "Camera.h"
#include "Material.h"
#include <memory>
class Entity
{
private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> mat;
public:
	Entity();
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat);
	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> mat);

	// Note: in general, a class should not delete an object it didn't make
	~Entity();

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();
	void Draw(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, std::shared_ptr<Camera> cam);
};

