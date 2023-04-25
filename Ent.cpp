#include "Ent.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

using namespace DirectX;
using namespace std;
using namespace Microsoft::WRL;

/// <summary>
/// I initialize multiple ents with a default constructor in Game.h because I have no info on their meshes and materials yet, 
/// so while this constructor does nothing, I need it or else there's going to be a Compiler Error C2512
/// </summary>
Ent::Ent() {
}

/// <summary>
/// Set this ent's properties
/// </summary>
/// <param name="mesh">- shape</param>
/// <param name="mat">- appearance</param>
Ent::Ent(shared_ptr<Mesh> mesh, shared_ptr<Material> mat)
{
    this->mesh = mesh;
	this->mat = mat;
}

/// <returns>The pointer to this ent's transform</returns>
Transform* Ent::GetTf()
{
	return &tf;
}

std::shared_ptr<Mesh> Ent::GetMesh()
{
	return mesh;
}

/// <returns>This ent's material</returns>
std::shared_ptr<Material> Ent::GetMat()
{
	return mat;
}

/// <summary>
/// Draw this entity's shape in the world and paint it with its material 
/// </summary>
/// <param name="cam">- the camera to draw this entity in relation to</param>
void Ent::Draw(shared_ptr<Cam> cam)
{
	shared_ptr<SimpleVertexShader> vs = mat->GetVertexShader();
	vs->SetMatrix4x4("world", tf.GetWorldMatrix());
	vs->SetMatrix4x4("view", cam->GetView());
	vs->SetMatrix4x4("proj", cam->GetProj());
	vs->SetMatrix4x4("worldIT", tf.GetWorldInverseTransposeMatrix());
	vs->CopyAllBufferData();

	shared_ptr<SimplePixelShader> ps = mat->GetPixelShader();
	ps->SetFloat4("tint", mat->GetColorTint());
	ps->SetFloat3("camPos", cam->GetPos());
	ps->CopyAllBufferData();

	mat->PrepareMaterial();

	mesh->Draw();
}
