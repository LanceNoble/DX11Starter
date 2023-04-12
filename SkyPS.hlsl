SamplerState CubeSampler : register(s1);
TextureCube Cube : register(t4);


struct SkyVertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

float4 main(SkyVertexToPixel input) : SV_TARGET
{
    return Cube.Sample(CubeSampler, input.sampleDir);
}