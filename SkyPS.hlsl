SamplerState CubeSampler : register(s0);
TextureCube Cube : register(t0);


struct SkyVertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

float4 main(SkyVertexToPixel input) : SV_TARGET
{
    return Cube.Sample(CubeSampler, input.sampleDir);
}