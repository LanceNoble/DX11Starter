#include "Lighting.hlsli"

cbuffer ExternalData : register(b2)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
    matrix view;
    matrix proj;
}

struct SkyVertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

SkyVertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    SkyVertexToPixel output;

    // can't edit variables from a constant buffer, so make a copy of it here
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    matrix vp = mul(proj, viewNoTranslation);
    // We assume the sky box is at the origin alongside the camera, so we can skip the world matrix
    output.screenPosition = mul(vp, float4(input.localPosition, 1.0f));
    output.screenPosition.z = output.screenPosition.w;
    
    output.sampleDir = input.localPosition;
    //output.sampleDir = mul(world, float4(input.localPosition, 1)).xyz;
    return output;
}