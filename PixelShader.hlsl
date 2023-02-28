#include "Lighting.hlsli"

//must set proper compiler options for every new shader added
cbuffer ExternalData : register(b0)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
	float4 colorTint;
	float3 cameraPosition;
	//float3 lightDirection;
	//float4 lightColor;
    float roughness;
    float3 ambience;
    Light directionalLight1;
}

float DiffuseBRDF(float3 normal, float3 dirToLight) {
	// surface normal should already be normalized
	// saturate() clamps result between 0 and 1 before returning
	// prevents dot product from being negative, which is bad
	return saturate(dot(normal,normalize(dirToLight)));
}

float SpecularBRDF(float3 normal, float3 lightDir, float3 viewVector) {
	// get reflection of light bouncing off surface
	float refl = reflect(lightDir, normal);
	
	//compare refl against view vec (do not want negative result), raising it to a very high power
	// to ensure the falloff to zero is quick
	float specular = saturate(dot(refl, viewVector));
	specular = pow(specular, 256);
	//specular = pow(specular, 128);
	//specular = pow(specular, 10);
	return specular;
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
	
	// Basic Diffuse Lighting Calculation
	// Calculate normalized direction to light
		// Negate light's direction and normalize it
    float3 dirToLight = normalize(-directionalLight1.Direction);
	// Calculate diffuse amount for light via N dot L diffuse lighting equation
    float diffuseAmount = DiffuseBRDF(input.normal, dirToLight);
	
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 viewVector = normalize(cameraPosition - input.worldPosition);
	float3 reflectionVector = reflect(directionalLight1.Direction, input.normal);
	float spec = pow(saturate(dot(reflectionVector, viewVector)), specExponent);
    //float3 light = colorTint * (diffuseAmount + spec); // tint specular
    //float3 light = colorTint * diffuseAmount + spec; // don't tint specular
	
	// Calculate final pixel color
		// based on surface's color, light's diffusion amount, light's color, and ambient color
    //float3 finalPixelColor = (diffuseAmount * directionalLight1.Color * colorTint) + (ambience * colorTint);
    float3 finalPixelColor = (directionalLight1.Color * colorTint * diffuseAmount + spec) + (ambience * colorTint);
	
    return float4(finalPixelColor, 1);
    //return float4(input.normal, 1);
    //return float4(ambience * colorTint, 1);
	
	
	
	// must normalize for dot product operations
	//float3 viewVector = normalize(cameraPosition - input.worldPosition);
	
	//float3 totalLightColor = ambience;
	//
	//totalLightColor += DiffuseBRDF(input.normal, -lightDirection) * lightColor;
	//totalLightColor += SpecularBRDF(input.normal, -lightDirection, viewVector) * lightColor;
	//
	//return float4(colorTint * totalLightColor, 1);
}