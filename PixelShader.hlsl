#include "Lighting.hlsli"

SamplerState BasicSampler : register(s0); // "s" registers for samplers
Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D NormalTexture : register(t1);


//must set proper compiler options for every new shader added
cbuffer ExternalData : register(b0)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
	float4 colorTint;
	float3 cameraPosition;
    float roughness;
    float3 ambience;
    float uvOffset;
    Light dir0;
    Light dir1;
    Light dir2;
    Light point0;
    Light point1;
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
    float specular = pow(saturate(dot(reflect(normalize(lightDir), normal), normalize(cameraPosition - pixWorldPos))), (1.0f - roughness) * MAX_SPECULAR_EXPONENT);
	return specular;
}

// Calculate light amount from one directional light
float3 HandleDirLight(Light dirLight, VertexToPixel input)
{
    float diffAm = DiffuseBRDF(input.normal, dirLight.Direction);
    float specAm = SpecularBRDF(input.normal, dirLight.Direction, input.worldPosition, roughness);
    
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
    
    return ((pointLight.Color * (diffAm + specAm)) * Attenuate(pointLight, input.worldPosition)) * pointLight.Intensity;
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
    input.uv.g += uvOffset;
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    
	// must re-normalize any interpolated vectors that were produced from rasterizer
	input.normal = normalize(input.normal);
    
    
	
    float3 totalLight = HandleDirLight(dir0, input) + HandleDirLight(dir1, input) + HandleDirLight(dir2, input);
    totalLight += HandlePoint(point0, input) + HandlePoint(point1, input);
	
    float3 finalPixelColor = surfaceColor * colorTint * (ambience + totalLight);
    
    return float4(finalPixelColor, 1);

}