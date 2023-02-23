// define constant buffer at the top
// important that every line of code is typed correctly and in this order

// ExternalData is an identifier used to signify the programmer's intent for this buffer, good for organization

// : register(b0) tells the shader which slot we're referring to when accessing the variables in this cbuffer
// binds resource or buffer to pipeline
// b0 = the buffer register at index 0
cbuffer ExternalData : register(b0) 
{
	// declare variables that hold the external data (data sent in from c++)
	// order at which they're declared matters (they define where in the buffer these variables will get their data)
	//float4 colorTint;
	matrix world;
	matrix viewMat;
	matrix projMat;
}

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{ 
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	//float4 color			: COLOR;        // RGBA color
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
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

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
	matrix wvp = mul(projMat, mul(viewMat, world));
	//output.screenPosition = mul(world, float4(input.localPosition, 1.0f));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	output.worldPosition = mul(world, float4(input.localPosition, 1.0f)).xyz;

	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	// - We don't need to alter it here, but we do need to send it to the pixel shader
	//output.color = colorTint;
	output.uv = input.uv;

	//this normal is in local space, not world space
	// to go from local -> world, we need a world matrix
	output.normal = normalize(mul((float3x3)world, input.normal));
	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}