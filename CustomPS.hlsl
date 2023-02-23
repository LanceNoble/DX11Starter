//must set proper compiler options for every new shader added
cbuffer ExternalData : register(b0)
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
	float4 colorTint;
	float3 cameraPosition;
	float3 ambientColor;
	float3 lightDirection;
	float4 lightColor;
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;	// XYZW position (System Value Position)
	//float4 color			: COLOR;        // RGBA color
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 worldPosition : POSITION;
};

float DiffuseBRDF(float3 normal, float3 dirToLight) {
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

	// must normalize for dot product operations
	float3 viewVector = normalize(cameraPosition - input.worldPosition);

	float3 totalLightColor = ambientColor;

	//totalLightColor += DiffuseBRDF(input.normal, -lightDirection) * lightColor * colorTint;
	totalLightColor += DiffuseBRDF(input.normal, -lightDirection) * lightColor;
	totalLightColor += SpecularBRDF(input.normal, -lightDirection, viewVector) * lightColor;

	//totalLightColor += DiffuseBRDF(input.normal, -lightDirection2) * light2Color * colorTint;
	//totalLightColor += SpecularBRDF(input.normal, -lightDirection2, viewVector) * light2Color;

	return float4(colorTint * totalLightColor, 1);
}