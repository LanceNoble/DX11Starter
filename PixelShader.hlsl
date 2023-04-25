#include "Lighting.hlsli"

Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);
SamplerState Sampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

//must set proper compiler options for every new shader added
cbuffer ExternalData : register(b1)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
	float4 tint;
	float3 camPos;
    Light dir;
    //Light pt;
    //Light spot;
}

// Calculate light amount from one directional light
float3 HandleDirLight(Light dirLight, VertexToPixel input, float metalness, float3 specColor, float3 surfaceColor, float roughness)
{
    float diffAm = DiffusePBR(input.normal, -dirLight.Direction);
    float3 F;
    float specAm = MicrofacetBRDF(input.normal, normalize(-dirLight.Direction), normalize(camPos - input.worldPosition), roughness, specColor, F);
    float3 balancedDiff = DiffuseEnergyConserve(diffAm, F, metalness);
    
    return (balancedDiff * surfaceColor + specAm) * dirLight.Intensity * dirLight.Color;
}

// Make point and spot lights weaken with distance
float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
    
}

// Calculate light amount from one point light
float3 HandlePoint(Light pointLight, VertexToPixel input, float metalness, float3 specColor, float3 surfaceColor, float roughness)
{
    float3 direction = normalize(pointLight.Position - input.worldPosition);
    
    float diffAm = DiffusePBR(input.normal, direction);
    float3 F;
    float specAm = MicrofacetBRDF(input.normal, normalize(direction), normalize(camPos - input.worldPosition), roughness, specColor, F);
    float3 balancedDiff = DiffuseEnergyConserve(diffAm, F, metalness);
    
    return (balancedDiff * surfaceColor + specAm) * pointLight.Intensity * pointLight.Color * Attenuate(pointLight, input.worldPosition);
}

float3 HandleSpot(Light spot, VertexToPixel input, float metalness, float3 specColor, float3 surfaceColor, float roughness)
{
    float3 dir = normalize(spot.Position - input.worldPosition);
    
    float diffAm = DiffusePBR(input.normal, dir);
    float3 F;
    float specAm = MicrofacetBRDF(input.normal, normalize(dir), normalize(camPos - input.worldPosition), roughness, specColor, F);
    float3 balancedDiff = DiffuseEnergyConserve(diffAm, F, metalness);
    
    float angleBtwn = cos(spot.SpotFalloff / 2);
    float dotProduct = dot(normalize(spot.Direction), dir);
    if (dotProduct < angleBtwn)
    {
        return (balancedDiff * surfaceColor + specAm) * spot.Intensity * spot.Color * Attenuate(spot, input.worldPosition);
    }
    return float3(0,0,0);
}


// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    input.shadowMapPos /= input.shadowMapPos.w;
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; 
    float distToLight = input.shadowMapPos.z;
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler,shadowUV,distToLight).r;
    
    float3 unpackedNormal = NormalMap.Sample(Sampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    float3 albedoColor = pow(Albedo.Sample(Sampler, input.uv).rgb, 2.2f);
    float roughness = RoughnessMap.Sample(Sampler, input.uv).r;
    float metalness = MetalnessMap.Sample(Sampler, input.uv).r;
    
    // Specular color determination
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use oflerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specColor = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
    
    // Gram-Schmidt orthonormalize process for making the normal and tanget orthogonal
	// must re-normalize any interpolated vectors that were produced from rasterizer
	input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));
    float3 biTan = cross(input.tangent, input.normal);
    float3x3 TBN = float3x3(input.tangent, biTan, input.normal);
    
    // Now rotate the unpacked normal to transform normal map normal from tangent space to world space
    // Assumes that input.normal is the normal later in the shader
    input.normal = mul(unpackedNormal, TBN); // Note the multiplication order
    
    float3 totalLight;
    totalLight = HandleDirLight(dir, input, metalness, specColor, albedoColor, roughness) * (shadowAmount + 0.2);
    //totalLight += HandlePoint(pt, input, metalness, specColor, albedoColor, roughness);
    //totalLight += HandleSpot(spot, input, metalness, specColor, albedoColor, roughness);
	
    return float4(pow(albedoColor * tint.rgb * totalLight, 1.0f / 2.2f), 1);
}