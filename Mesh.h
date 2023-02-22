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
		void MakeVB(Vertex*, int, Microsoft::WRL::ComPtr<ID3D11Device>);
		void MakeIB(unsigned int*, int, Microsoft::WRL::ComPtr<ID3D11Device>);
	public:
		Mesh(Vertex*, int, unsigned int*, int, Microsoft::WRL::ComPtr<ID3D11Device>, Microsoft::WRL::ComPtr<ID3D11DeviceContext>);
		Mesh(const wchar_t*, Microsoft::WRL::ComPtr<ID3D11Device>, Microsoft::WRL::ComPtr<ID3D11DeviceContext>);
		void Draw();
};
