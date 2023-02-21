#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <fstream>
#include <DirectXMath.h>
#include <vector>
#include "Vertex.h"
class Mesh
{
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
		int indexCount = 0;
		int vertexCount = 0;

	public:

		Mesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);
		Mesh(const wchar_t* fileName, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);
		~Mesh();
		ID3D11Buffer* GetVertexBuffer();
		ID3D11Buffer* GetIndexBuffer();
		int GetIndexCount();
		void Draw();
};
