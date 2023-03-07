#include "Entity.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

using namespace DirectX;
using namespace std;

Entity::Entity() {
	mesh = NULL;
}

/// <summary>
/// saves a pointer to the entity's mesh
/// </summary>
/// <param name="mesh">pointer to mesh</param>
Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat)
{
    this->mesh = mesh;
	this->mat = mat;
}

std::shared_ptr<Material> Entity::GetMaterial()
{
	return mat;
}

void Entity::SetMaterial(std::shared_ptr<Material> mat)
{
	this->mat = mat;
}

/// <summary>
/// does nothing
/// </summary>
Entity::~Entity()
{

}

/// <summary>
/// get the pointer to this entity's mesh
/// </summary>
/// <returns>the pointer to this entity's mesh</returns>
std::shared_ptr<Mesh> Entity::GetMesh()
{
    return std::shared_ptr<Mesh>();
}

/// <summary>
/// Get the pointer to this entity's transform
/// </summary>
/// <returns>the pointer to this entity's transform</returns>
Transform* Entity::GetTransform()
{
    return &transform;
}

/// <summary>
/// draw entity
/// </summary>
void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, std::shared_ptr<Camera> cam)
{
	static float uvOffset;
	uvOffset += 0.00005f;
	shared_ptr<SimpleVertexShader> vs = mat->GetVertexShader();
	// Very important that string parameters match the variable names in your cbuffer
	//vs->SetFloat4("colorTint", mat->GetColorTint());
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("viewMat", cam->GetViewMat());
	vs->SetMatrix4x4("projMat", cam->GetProjMat());
	vs->SetMatrix4x4("worldInvTranspose", transform.GetWorldInverseTransposeMatrix());
	vs->CopyAllBufferData();

	shared_ptr<SimplePixelShader> ps = mat->GetPixelShader();
	ps->SetFloat4("colorTint", mat->GetColorTint());
	ps->SetFloat3("cameraPosition", cam->transform.GetPosition());
	//ps->SetFloat3("lightDirection", XMFLOAT3(0.0f,0.0f,1.0f));
	//ps->SetFloat4("lightColor", XMFLOAT4(1.0f,1.0f,1.0f,1.0f));
	ps->SetFloat("roughness", mat->GetRoughness());
	ps->SetFloat3("ambience", mat->GetAmbience());
	ps->SetFloat("uvOffset", uvOffset);
	ps->CopyAllBufferData();

	mat->PrepareMaterial();

	mesh->Draw();
}
