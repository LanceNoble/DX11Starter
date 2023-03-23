cbuffer ExternalData : register(b0)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
    matrix viewMat;
    matrix projMat;
    matrix worldInvTranspose;

}

struct VertexShaderInput
{
    float3 localPosition : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
};

VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel output;

    matrix viewNoTranslation = viewMat;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    matrix vp = mul(projMat, viewMat);
    output.screenPosition = mul(vp, float4(input.localPosition, 1.0f));
    
    output.screenPosition.z = output.screenPosition.w;
	
    return output;
}