//https://www.geeksforgeeks.org/include-guards-in-c/
#ifndef __GGP_SHADER_LIGHTING_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_LIGHTING_INCLUDES__

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_SPECULAR_EXPONENT 256.0f



// ALL of your code pieces (structs, functions, etc.) go here!

// order of variable declarations matter
// because we're sending this to a constant buffer in the pixel shader
struct Light
{
    int Type; // Which kind of light? 0, 1 or 2 (see above)
    float3 Direction; // Directional and Spot lights need a direction
    float Range; // Point and Spot lights have a max range for attenuation
    float3 Position; // Point and Spot lights have a position in space
    float Intensity; // All lights need an intensity
    float3 Color; // All lights need a color
    float SpotFalloff; // Spot lights need a value to define their “cone” size
    float3 Padding; // Purposefully padding to hit the 16-byte boundary

};

struct VertexShaderInput
{
    float3 localPosition : POSITION; 
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
};

#endif