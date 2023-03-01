#include "Lighting.hlsli"

//must set proper compiler options for every new shader added
cbuffer ExternalData : register(b0)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
	float4 colorTint;
	float3 cameraPosition;
    float roughness;
    float3 ambience;
    Light directionalLight1;
    Light dirLight2;
    Light dirLight3;
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
    float specular = pow(saturate(dot(reflect(lightDir, normal), normalize(cameraPosition - pixWorldPos))), (1.0f - roughness) * MAX_SPECULAR_EXPONENT);
	return specular;
}

float3 HandleDirLight(Light dirLight, VertexToPixel input)
{
    float diffAm = DiffuseBRDF(input.normal, dirLight.Direction);
    float specAm = SpecularBRDF(input.normal, dirLight.Direction, input.worldPosition, roughness);
    
    // To tint the specular, surround "diffAm + specAm" in another inner set of parentheses
    float3 pixColor = (dirLight.Color * colorTint * diffAm + specAm);
    return pixColor;
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
    
}

float3 HandlePoint(Light pointLight, VertexToPixel input)
{
    float3 direction = normalize(input.worldPosition - pointLight.Position);
    float diffAm = DiffuseBRDF(input.normal, direction);
    float specAm = SpecularBRDF(input.normal, direction, input.worldPosition, roughness);
    
    float3 pixColor = (pointLight.Color * colorTint * diffAm + specAm);
    pixColor *= Attenuate(pointLight, input.worldPosition);
    return pixColor;
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
	// must re-normalize any interpolated vectors that were produced from rasterizer
	input.normal = normalize(input.normal);
	
    float diffuseAmount = DiffuseBRDF(input.normal, directionalLight1.Direction);

    float specAmount = SpecularBRDF(input.normal, directionalLight1.Direction, input.worldPosition, roughness);
	
    float3 totalLight = HandleDirLight(directionalLight1, input) + HandleDirLight(dirLight2, input) + HandleDirLight(dirLight3, input);
    totalLight += HandlePoint(point0, input) + HandlePoint(point1, input);
	
    // float3 finalPixelColor = (directionalLight1.Color * colorTint * diffuseAmount + specAmount) + (ambience * colorTint);
    float3 finalPixelColor = totalLight + (ambience * colorTint);
    return float4(finalPixelColor, 1);
}