#include "Lighting.hlsli"

SamplerState BasicSampler : register(s0); // "s" registers for samplers
Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);


//must set proper compiler options for every new shader added
cbuffer ExternalData : register(b0)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
	float4 tint;
	float3 camPos;
    float roughness;
    float3 ambience;
    Light dir;
    Light pt;
    Light spot;
}

// Cut the specular if the diffuse contribution is zero
// - any() returns 1 if any component of the param is non-zero
// - In this case, diffuse is a single float value
// - Meaning any() returns 1 if diffuse itself is non-zero
// - In other words:
// - If the diffuse amount is 0, any(diffuse) returns 0
// - If the diffuse amount is != 0, any(diffuse) returns 1
// - So when diffuse is 0, specular becomes 0
float cutSpec(float spec, float diffuse)
{
    spec *= any(diffuse);
    return spec;
}

// Lighting equation
// Calculates the dot product between the normalized surface normal of the pixel
// and the normalized + negated direction of a light, then clamps the result between
// 0 and 1 to find the appropriate diffusal amount
float DiffuseBRDF(float3 normal, float3 lightDir) {
	return saturate(dot(normal,normalize(-lightDir)));
}

// Lighting equation
// Calculates a reflection vector from the direction of the light and the normalized surface normal
// Normalizes the vector pointing to the camera from the pixel's world position
// Calculates the dot product between the two vectors, clamps the result between 0 and 1, and
// raises it to a specular exponent power determined by the entity's material's roughness
// to find the appropriate specular amount
float SpecularBRDF(float3 normal, float3 lightDir, float3 pixWorldPos, float roughness) {
    float specular = 0;
    
    // Conditional accounts for the spec exp being 0 and giving a result of 1, which causes the spec to be pure white
    if (MAX_SPECULAR_EXPONENT > 0.05)
        specular = pow(saturate(dot(reflect(normalize(lightDir), normal), normalize(camPos - pixWorldPos))), (1.0f - roughness) * MAX_SPECULAR_EXPONENT);
    
    return specular;
}

// Calculate light amount from one directional light
float3 HandleDirLight(Light dirLight, VertexToPixel input)
{
    float diffAm = DiffuseBRDF(input.normal, dirLight.Direction);
    float specAm = SpecularBRDF(input.normal, dirLight.Direction, input.worldPosition, roughness);
    
    cutSpec(specAm, diffAm);
    
    // To tint the specular, surround "diffAm + specAm" in another inner set of parentheses
    return (dirLight.Color * (diffAm + specAm)) * dirLight.Intensity;
}

// Make point lights weaken with distance
float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
    
}

// Calculate light amount from one point light
float3 HandlePoint(Light pointLight, VertexToPixel input)
{
    float3 direction = normalize(input.worldPosition - pointLight.Position);
    float diffAm = DiffuseBRDF(input.normal, direction);
    float specAm = SpecularBRDF(input.normal, direction, input.worldPosition, roughness);
    cutSpec(specAm, diffAm);
    return ((pointLight.Color * (diffAm + specAm)) * Attenuate(pointLight, input.worldPosition)) * pointLight.Intensity;
}

float3 HandleSpot(Light spot, VertexToPixel input)
{
    float3 dir = normalize(input.worldPosition - spot.Position);
    float diffAm = DiffuseBRDF(input.normal, dir);
    float specAm = SpecularBRDF(input.normal, dir, input.worldPosition, roughness);
    cutSpec(specAm, diffAm);
    
    //float angleBtwn = cos();
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
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal); // Don't forget to normalize (because of rasterizer interpolation)

    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    
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
    
    float3 totalLight = HandleDirLight(dir, input);
    totalLight += HandlePoint(pt, input);
    //totalLight += HandleSpot(spot, input);
	
    float3 finalPixelColor = surfaceColor * tint.rgb * (ambience + totalLight);
    
    return float4(finalPixelColor, 1);
}