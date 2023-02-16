#include "Entity.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

using namespace DirectX;

Entity::Entity() {
	mesh = NULL;
}

/// <summary>
/// saves a pointer to the entity's mesh
/// </summary>
/// <param name="mesh">pointer to mesh</param>
Entity::Entity(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
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
void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, float* colorTint, std::shared_ptr<Camera> cam)
{
	// we'll use this to create that constant buffer resource in GPU memory after defining the buffers in C++ and shader
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;

    // The entity already has a pointer to a mesh, whose constructor already went through the process of creating the vertex and index buffers
    // All we have to focus on in this method is creating the constant buffer I assume
    // Then call the mesh's draw method
	
	//constant buffers have a size restriction unlike index and vertex (must be multiple of 16)
	// Get size as the next multiple of 16 (instead of hardcoding size)
	unsigned int size = sizeof(VertexShaderExternalData);
	size = (size + 15) / 16 * 16;

	// describe constant buffer
	D3D11_BUFFER_DESC cbDesc = {}; // set struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // saying that we intend to bind this buffer as a constant buffer
	cbDesc.ByteWidth = size; // must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // saying that we intend to write to this buffer from C++ but never read from it
	cbDesc.Usage = D3D11_USAGE_DYNAMIC; // saying that this buffer will be read from the GPU and written to by the CPU

	// don't need to specify initial data (the pointer to system memory stuff) because constant buffer data will change CONSTANTLY because of things like changes in lighting
	// now to actually create the constant buffer for real
	device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());

	// local variable of new struct
	// used for collecting data you intend to copy to the constant buffer
	// now we are ready to copy
	VertexShaderExternalData vsData;
	//vsData.colorTint = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	//vsData.offset = XMFLOAT3(0.25f, 0.0f, 0.0f);
	vsData.colorTint = XMFLOAT4(colorTint[0], colorTint[1], colorTint[2], colorTint[3]);
	//vsData.worldMatrix = XMFLOAT3(this->offset[0], this->offset[1], this->offset[2]);
	vsData.worldMatrix = transform.GetWorldMatrix();
	vsData.viewMat = cam->GetViewMat();
	vsData.projMat = cam->GetProjMat();
	//vsData.offset = offset;
	//colorTint = vsData.colorTint;
	//offset = vsData.offset;

	// copy the data
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {}; // MAPPED SUBRESOURCE - pointer to resource's memory after mapping
	// lock resource so gpu can't use it and we get breathing room to operate on it
	// param1, resource we're mapping. param2, tell d3d that it can safely erase the data that's currently on the buffer
	deviceContext->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	// now copy the data onto mappedBuffer
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	// now unlock the resource and let the gpu use it
	deviceContext->Unmap(vsConstantBuffer.Get(), 0);

	// next, bind the constant buffer to the correct place in the pipeline
	deviceContext->VSSetConstantBuffers(
		0, // which slot (register) to bind the buffer to
		1, // how many are we activating? can do multiple at once
		vsConstantBuffer.GetAddressOf()); // arra of buffers (or the address of one)

	mesh->Draw();
}
