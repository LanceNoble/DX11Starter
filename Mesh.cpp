//#include <wrl/client.h>

// Order of include statements matter: if you include mesh before vertex and dxcore and mesh.h uses variables from vertex and dxcore, those variables technically still won't exist yet 
//#include "Vertex.h"
//#include "DXCore.h"
#include "Mesh.h"


/// <summary>
/// Creates a vertex and index buffer for rendering shapes on the screen
/// </summary>
/// <param name="vertices">vertex data</param>
/// <param name="vertexCount">number of vertices</param>
/// <param name="indices">index data</param>
/// <param name="indexCount">number of indices</param>
/// <param name="device">d3d device object</param>
/// <param name="deviceContext">d3d context object</param>
Mesh::Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext)
{
	this->indexCount = indexCount;
	this->deviceContext = deviceContext;

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices;

	device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * this->indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices;

	device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
};

/// <summary>
/// We have com pointers so nothing to do here yet
/// </summary>
Mesh::~Mesh()
{

};

/// <summary>
/// Retrieve vertex buffer
/// </summary>
/// <returns>vertex buffer</returns>
ID3D11Buffer* Mesh::GetVertexBuffer()
{
	return vertexBuffer.Get();
};

/// <summary>
/// Retrieve index buffer
/// </summary>
/// <returns>index buffer</returns>
ID3D11Buffer* Mesh::GetIndexBuffer()
{
	return indexBuffer.Get();
};

/// <summary>
/// Get index count
/// </summary>
/// <returns>index count</returns>
int Mesh::GetIndexCount()
{
	return 0;
};

/// <summary>
/// Draws the mesh
/// </summary>
void Mesh::Draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	deviceContext->DrawIndexed(this->indexCount, 0, 0);
};
