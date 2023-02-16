#pragma once

#include <DirectXMath.h>

// used for defining the C++ struct that should match the layout of our vertex shader's cbuffer (this is necessary)

// every new buffer in the shader needs a new buffer in c++ that corresponds to it
// make sure no identifiers here match the identifiers in the shader
struct VertexShaderExternalData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMat;
	DirectX::XMFLOAT4X4 projMat;
};