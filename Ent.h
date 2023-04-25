#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Cam.h"
#include "Material.h"

/// <summary>
/// Entity class
/// </summary>
class Ent
{
private:
	Transform tf;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> mat;
public:
	Ent();
	Ent(std::shared_ptr<Mesh>, std::shared_ptr<Material>);
	Transform* GetTf();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMat();
	void Draw(std::shared_ptr<Cam>);
};

